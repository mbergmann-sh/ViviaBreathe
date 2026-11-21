// ViviaBreathe mobil – Erinnerungen, Wecker und geplantes Einschlafen über den AlarmManager
//
// Die App (C++) plant die Termine der nächsten 14 Tage und übergibt sie hier (replaceAll). Zur Zeit
// meldet sich ReminderReceiver – auch wenn ViviaBreathe im Hintergrund ist oder beendet wurde:
//   * Wecker (und automatisches Einschlafen): startet PlaybackService mit der Wiedergabeliste
//   * sonst: Benachrichtigung; Antippen öffnet die App (OpenActivity) an der passenden Stelle
// Der Plan steht zusätzlich in den (app-eigenen) SharedPreferences, damit BootReceiver ihn nach
// einem Neustart wieder stellen kann. Wecker nutzen setAlarmClock (pünktlich, Symbol in der
// Statusleiste); dafür braucht es ab Android 12 das Recht „Wecker und Erinnerungen“.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.json.JSONArray;
import org.json.JSONObject;

public class Reminders {
    static final String CHANNEL_SOUND = "erinnerungen";
    static final String CHANNEL_SILENT = "erinnerungen_still";
    static final String PREFS = "viviabreathe_reminders";
    static final String KEY_PLAN = "plan";
    private static final ExecutorService WORKER = Executors.newSingleThreadExecutor();

    public static void ensureChannels(Context ctx, String nameSound, String nameSilent, String description) {
        if (Build.VERSION.SDK_INT < 26) return;
        NotificationManager nm = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel a = new NotificationChannel(CHANNEL_SOUND, nameSound, NotificationManager.IMPORTANCE_HIGH);
        a.setDescription(description);
        a.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        nm.createNotificationChannel(a);
        NotificationChannel b = new NotificationChannel(CHANNEL_SILENT, nameSilent, NotificationManager.IMPORTANCE_DEFAULT);
        b.setDescription(description);
        b.setSound(null, null);
        b.enableVibration(false);
        b.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        nm.createNotificationChannel(b);
    }

    // Android 13+: Erlaubnis für Benachrichtigungen erfragen (einmalig; danach in den Systemeinstellungen)
    public static boolean ensurePermission(Context ctx) {
        if (Build.VERSION.SDK_INT < 33) return true;
        if (ctx.checkSelfPermission("android.permission.POST_NOTIFICATIONS") == PackageManager.PERMISSION_GRANTED) return true;
        if (ctx instanceof Activity)
            ((Activity) ctx).requestPermissions(new String[] { "android.permission.POST_NOTIFICATIONS" }, 4712);
        return false;
    }

    public static boolean permissionGranted(Context ctx) {
        if (Build.VERSION.SDK_INT < 33) return true;
        return ctx.checkSelfPermission("android.permission.POST_NOTIFICATIONS") == PackageManager.PERMISSION_GRANTED;
    }

    public static boolean exactAllowed(Context ctx) {
        if (Build.VERSION.SDK_INT < 31) return true;
        AlarmManager am = (AlarmManager) ctx.getSystemService(Context.ALARM_SERVICE);
        return am != null && am.canScheduleExactAlarms();
    }

    public static void openExactSettings(Context ctx) {
        if (Build.VERSION.SDK_INT < 31) return;
        Intent i = new Intent(android.provider.Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM,
                              android.net.Uri.parse("package:" + ctx.getPackageName()));
        if (!(ctx instanceof Activity)) i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try { ctx.startActivity(i); } catch (Exception e) { /* Einstellung nicht vorhanden */ }
    }

    private static PendingIntent alarmIntent(Context ctx, JSONObject o, String publicText) {
        Intent i = new Intent(ctx, ReminderReceiver.class);
        int id = o.optInt("id");
        i.setAction("de.mbergmann.viviabreathe.REMINDER." + id);
        i.putExtra("id", id);
        i.putExtra("kind", o.optString("kind"));
        i.putExtra("ref", o.optLong("ref"));
        i.putExtra("user", o.optLong("user"));
        i.putExtra("title", o.optString("title"));
        i.putExtra("text", o.optString("text"));
        i.putExtra("publicText", publicText);
        i.putExtra("sound", o.optBoolean("sound", true));
        JSONObject play = o.optJSONObject("play");
        if (play != null) i.putExtra("play", play.toString());
        return PendingIntent.getBroadcast(ctx, id, i, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
    }

    private static void setAlarm(Context ctx, JSONObject o, String publicText) {
        AlarmManager am = (AlarmManager) ctx.getSystemService(Context.ALARM_SERVICE);
        long at = o.optLong("at");
        PendingIntent pi = alarmIntent(ctx, o, publicText);
        boolean exact = Build.VERSION.SDK_INT < 31 || am.canScheduleExactAlarms();
        try {
            if (exact && "alarm".equals(o.optString("kind"))) {
                PendingIntent show = PendingIntent.getActivity(ctx, o.optInt("id"),
                    OpenActivity.intentFor(ctx, "alarmInfo", o.optLong("ref"), o.optLong("user")),
                    PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
                am.setAlarmClock(new AlarmManager.AlarmClockInfo(at, show), pi);
            } else if (exact) {
                am.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pi);
            } else {
                am.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pi);
            }
        } catch (SecurityException e) {
            am.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pi);
        }
    }

    // Alle bisher gestellten Termine durch 'planJson' ersetzen
    public static void replaceAll(Context ctx, String planJson, String publicText) {
        final Context app = ctx.getApplicationContext();
        WORKER.execute(() -> replaceAllNow(app, planJson, publicText));
    }

    private static void replaceAllNow(Context ctx, String planJson, String publicText) {
        SharedPreferences prefs = ctx.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        AlarmManager am = (AlarmManager) ctx.getSystemService(Context.ALARM_SERVICE);
        try {
            JSONArray old = new JSONArray(prefs.getString(KEY_PLAN, "[]"));
            for (int k = 0; k < old.length(); ++k) am.cancel(alarmIntent(ctx, old.getJSONObject(k), ""));
        } catch (Exception e) { /* alter Plan unlesbar: nichts zu streichen */ }
        JSONArray kept = new JSONArray();
        long now = System.currentTimeMillis();
        try {
            JSONArray plan = new JSONArray(planJson);
            for (int k = 0; k < plan.length(); ++k) {
                JSONObject o = plan.getJSONObject(k);
                if (o.optLong("at") <= now) continue;
                o.put("publicText", publicText);
                setAlarm(ctx, o, publicText);
                kept.put(o);
            }
        } catch (Exception e) { /* ungültiger Plan */ }
        prefs.edit().putString(KEY_PLAN, kept.toString()).apply();
    }

    // nach einem Neustart: gespeicherten Plan erneut stellen
    static void restore(Context ctx) {
        SharedPreferences prefs = ctx.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        try {
            JSONArray plan = new JSONArray(prefs.getString(KEY_PLAN, "[]"));
            long now = System.currentTimeMillis();
            for (int k = 0; k < plan.length(); ++k) {
                JSONObject o = plan.getJSONObject(k);
                if (o.optLong("at") <= now) continue;
                setAlarm(ctx, o, o.optString("publicText", "ViviaBreathe"));
            }
        } catch (Exception e) { /* nichts zu tun */ }
    }

    // Schlummern: denselben Wecker in 'minutes' Minuten noch einmal (ohne den Plan zu ändern)
    static void snooze(Context ctx, String playJson, int minutes) {
        try {
            JSONObject play = new JSONObject(playJson);
            JSONObject o = new JSONObject();
            o.put("id", 0x7ff00000 | (int) (play.optLong("ref") & 0xFFFF));
            o.put("at", System.currentTimeMillis() + Math.max(1, minutes) * 60000L);
            o.put("kind", "alarm");
            o.put("ref", play.optLong("ref"));
            o.put("user", play.optLong("user"));
            o.put("title", play.optString("title"));
            o.put("text", play.optString("text"));
            o.put("play", play);
            setAlarm(ctx, o, "ViviaBreathe");
        } catch (Exception e) { /* ohne Schlummern */ }
    }

    static void show(Context ctx, int id, String kind, long ref, long user, String title, String text, String publicText, boolean sound) {
        if (!permissionGranted(ctx)) return;
        NotificationManager nm = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
        PendingIntent content = PendingIntent.getActivity(ctx, id, OpenActivity.intentFor(ctx, kind == null ? "" : kind, ref, user),
                                                          PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        String channel = sound ? CHANNEL_SOUND : CHANNEL_SILENT;
        int icon = ctx.getResources().getIdentifier("ic_stat_viviabreathe", "drawable", ctx.getPackageName());
        if (icon == 0) icon = android.R.drawable.ic_popup_reminder;
        // Sperrbildschirm: nur „ViviaBreathe – Erinnerung“, keine Einzelheiten
        Notification pub = new Notification.Builder(ctx, channel)
            .setSmallIcon(icon).setContentTitle("ViviaBreathe").setContentText(publicText).build();
        Notification.Builder b = new Notification.Builder(ctx, channel)
            .setSmallIcon(icon)
            .setContentTitle(title)
            .setContentText(text)
            .setStyle(new Notification.BigTextStyle().bigText(text))
            .setCategory(Notification.CATEGORY_REMINDER)
            .setVisibility(Notification.VISIBILITY_PRIVATE)
            .setPublicVersion(pub)
            .setContentIntent(content)
            .setAutoCancel(true);
        nm.notify(id, b.build());
    }
}
