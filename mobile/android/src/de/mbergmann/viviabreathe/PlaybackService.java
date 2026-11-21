// ViviaBreathe mobil – Musik für Wecker und Einschlafen als Vordergrunddienst
//
// Spielt eine Wiedergabeliste (content://-Adressen aus der Dateiauswahl oder Dateipfade) auch bei
// ausgeschaltetem Bildschirm und wenn die App im Hintergrund ist oder beendet wurde:
//   * mode "alarm": wird sanft lauter, wiederholt die Liste, bis „Aus“ oder „Schlummern“ (höchstens
//     30 Minuten); Tonausgabe als Wecker (USAGE_ALARM)
//   * mode "sleep": spielt 'durationSec' lang und wird zum Ende über 'fadeOutSec' leiser
// Lässt sich keine Datei abspielen, nimmt der Wecker den eingebauten Weckton ('fallback').
// Zustand für die App: state() (JSON). Steuerung: start(), stopNow(), snooze().
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONObject;

public class PlaybackService extends Service {
    static final String CHANNEL = "musik";
    static final int NOTIFICATION_ID = 0x5A11;
    static final String ACTION_START = "de.mbergmann.viviabreathe.PLAY";
    static final String ACTION_STOP = "de.mbergmann.viviabreathe.STOP";
    static final String ACTION_SNOOZE = "de.mbergmann.viviabreathe.SNOOZE";
    static final long MAX_ALARM_MS = 30L * 60 * 1000;

    // Zustand für die App (aus dem Oberflächen-Thread gelesen)
    static volatile boolean sRunning = false;
    static volatile String sMode = "";
    static volatile String sTitle = "";
    static volatile String sTrack = "";
    static volatile long sRef = 0;
    static volatile long sEndAt = 0;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private MediaPlayer player;
    private JSONObject config = new JSONObject();
    private final List<String> files = new ArrayList<>();
    private int index = -1;
    private int errors = 0;
    private long startedAt = 0;
    private float volume = 0f;

    // ---------------------------------------------------------------- Steuerung (statisch, für C++)

    public static void start(Context ctx, String json) {
        Intent i = new Intent(ctx, PlaybackService.class);
        i.setAction(ACTION_START);
        i.putExtra("config", json);
        try {
            if (Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(i); else ctx.startService(i);
        } catch (Exception e) { /* aus dem Hintergrund nicht erlaubt */ }
    }

    // vom Wecker des Systems (ReminderReceiver); false = Dienst ließ sich nicht starten
    static boolean startFromAlarm(Context ctx, String json) {
        try {
            Intent i = new Intent(ctx, PlaybackService.class);
            i.setAction(ACTION_START);
            i.putExtra("config", json);
            if (Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(i); else ctx.startService(i);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static void stopNow(Context ctx) {
        Intent i = new Intent(ctx, PlaybackService.class);
        i.setAction(ACTION_STOP);
        try { ctx.startService(i); } catch (Exception e) { /* läuft nicht */ }
    }

    public static void snooze(Context ctx) {
        Intent i = new Intent(ctx, PlaybackService.class);
        i.setAction(ACTION_SNOOZE);
        try { ctx.startService(i); } catch (Exception e) { /* läuft nicht */ }
    }

    public static String state() {
        try {
            JSONObject o = new JSONObject();
            o.put("running", sRunning);
            o.put("mode", sMode);
            o.put("title", sTitle);
            o.put("track", sTrack);
            o.put("ref", sRef);
            o.put("remainingSec", sEndAt > 0 ? Math.max(0, (sEndAt - System.currentTimeMillis()) / 1000) : 0);
            return o.toString();
        } catch (Exception e) {
            return "{}";
        }
    }

    // ---------------------------------------------------------------- Dienst

    @Override
    public IBinder onBind(Intent intent) { return null; }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent == null ? "" : intent.getAction();
        if (ACTION_STOP.equals(action)) { finish(); return START_NOT_STICKY; }
        if (ACTION_SNOOZE.equals(action)) {
            Reminders.snooze(this, config.toString(), config.optInt("snoozeMin", 9));
            finish();
            return START_NOT_STICKY;
        }
        try {
            config = new JSONObject(intent.getStringExtra("config"));
        } catch (Exception e) {
            finish();
            return START_NOT_STICKY;
        }
        goForeground();
        begin();
        return START_NOT_STICKY;
    }

    private boolean isAlarm() { return "alarm".equals(config.optString("mode")); }

    private void goForeground() {
        NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        if (Build.VERSION.SDK_INT >= 26) {
            NotificationChannel ch = new NotificationChannel(CHANNEL, config.optString("channelName", "Wecker und Einschlafmusik"),
                                                             NotificationManager.IMPORTANCE_HIGH);
            ch.setSound(null, null);          // Ton kommt von der Musik selbst
            ch.enableVibration(false);
            nm.createNotificationChannel(ch);
        }
        int icon = getResources().getIdentifier("ic_stat_viviabreathe", "drawable", getPackageName());
        if (icon == 0) icon = android.R.drawable.ic_lock_idle_alarm;
        String kind = isAlarm() ? "alarm" : "sleepRunning";
        PendingIntent content = PendingIntent.getActivity(this, 1, OpenActivity.intentFor(this, kind, config.optLong("ref"), config.optLong("user")),
                                                          PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        Intent stop = new Intent(this, PlaybackService.class).setAction(ACTION_STOP);
        PendingIntent stopPi = PendingIntent.getService(this, 2, stop, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        Notification.Builder b = new Notification.Builder(this, CHANNEL)      // ab Android 8 (minSdk 28)
            .setSmallIcon(icon)
            .setContentTitle(config.optString("title", "ViviaBreathe"))
            .setContentText(config.optString("text", ""))
            .setContentIntent(content)
            .setOngoing(true)
            .setVisibility(Notification.VISIBILITY_PUBLIC)      // „Aus“/„Schlummern“ auch auf dem Sperrbildschirm
            .setCategory(isAlarm() ? Notification.CATEGORY_ALARM : Notification.CATEGORY_SERVICE)
            .addAction(new Notification.Action.Builder(null, config.optString("stopLabel", "Aus"), stopPi).build());
        if (isAlarm() && config.optInt("snoozeMin", 0) > 0) {
            Intent sn = new Intent(this, PlaybackService.class).setAction(ACTION_SNOOZE);
            PendingIntent snPi = PendingIntent.getService(this, 3, sn, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            b.addAction(new Notification.Action.Builder(null, config.optString("snoozeLabel", "Schlummern"), snPi).build());
            b.setFullScreenIntent(content, true);
        }
        Notification n = b.build();
        if (Build.VERSION.SDK_INT >= 29)
            startForeground(NOTIFICATION_ID, n, ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK);
        else
            startForeground(NOTIFICATION_ID, n);
    }

    private void begin() {
        releasePlayer();
        files.clear();
        JSONArray a = config.optJSONArray("files");
        if (a != null) for (int k = 0; k < a.length(); ++k) if (!a.optString(k).isEmpty()) files.add(a.optString(k));
        if (config.optBoolean("shuffle", false)) Collections.shuffle(files);
        if (files.isEmpty() && isAlarm() && !config.optString("fallback").isEmpty()) files.add(config.optString("fallback"));
        if (files.isEmpty()) { finish(); return; }
        index = -1;
        errors = 0;
        volume = 0f;
        startedAt = System.currentTimeMillis();
        int dur = config.optInt("durationSec", 0);
        sEndAt = dur > 0 ? startedAt + dur * 1000L : 0;
        sMode = config.optString("mode");
        sTitle = config.optString("title");
        sRef = config.optLong("ref");
        sRunning = true;
        next();
        handler.removeCallbacks(tick);
        handler.post(tick);
    }

    private void next() {
        if (files.isEmpty()) { finish(); return; }
        index = (index + 1) % files.size();
        String f = files.get(index);
        releasePlayer();
        try {
            player = new MediaPlayer();
            player.setAudioAttributes(new AudioAttributes.Builder()
                .setUsage(isAlarm() ? AudioAttributes.USAGE_ALARM : AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build());
            player.setWakeMode(this, PowerManager.PARTIAL_WAKE_LOCK);
            if (f.contains("://")) player.setDataSource(this, Uri.parse(f)); else player.setDataSource(f);
            player.setOnCompletionListener(mp -> next());
            player.setOnErrorListener((mp, what, extra) -> { failed(); return true; });
            player.prepare();
            applyVolume();
            player.start();
            sTrack = Device.displayName(this, f);
            errors = 0;
        } catch (Exception e) {
            failed();
        }
    }

    // Datei lässt sich nicht abspielen: nächste versuchen, zuletzt den eingebauten Weckton
    private void failed() {
        ++errors;
        if (errors >= files.size()) {
            String fb = config.optString("fallback");
            if (isAlarm() && !fb.isEmpty() && !files.contains(fb)) {
                files.clear();
                files.add(fb);
                index = -1;
                errors = 0;
                handler.post(this::next);
                return;
            }
            finish();
            return;
        }
        handler.post(this::next);
    }

    private final Runnable tick = new Runnable() {
        @Override public void run() {
            if (!sRunning) return;
            long now = System.currentTimeMillis();
            long el = now - startedAt;
            float target = config.optInt("volume", 50) / 100f;
            long fadeIn = Math.max(1500, config.optInt("fadeInSec", 0) * 1000L);
            float v = el < fadeIn ? target * el / fadeIn : target;
            if (sEndAt > 0) {
                long fadeOut = config.optInt("fadeOutSec", 0) * 1000L;
                long left = sEndAt - now;
                if (left <= 0) { finish(); return; }
                if (fadeOut > 0 && left < fadeOut) v = Math.min(v, target * left / fadeOut);
            }
            if (isAlarm() && el > MAX_ALARM_MS) { finish(); return; }
            volume = v;
            applyVolume();
            handler.postDelayed(this, 250);
        }
    };

    private void applyVolume() {
        if (player == null) return;
        // gehörrichtig: leise Bereiche feiner
        float v = volume * volume;
        try { player.setVolume(v, v); } catch (Exception e) { /* Spieler nicht bereit */ }
    }

    private void releasePlayer() {
        if (player != null) {
            try { player.stop(); } catch (Exception e) { /* nicht gestartet */ }
            player.release();
            player = null;
        }
    }

    private void finish() {
        handler.removeCallbacks(tick);
        releasePlayer();
        sRunning = false;
        sMode = "";
        sTitle = "";
        sTrack = "";
        sEndAt = 0;
        sRef = 0;
        stopForeground(Service.STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    @Override
    public void onDestroy() {
        handler.removeCallbacks(tick);
        releasePlayer();
        sRunning = false;
        super.onDestroy();
    }
}
