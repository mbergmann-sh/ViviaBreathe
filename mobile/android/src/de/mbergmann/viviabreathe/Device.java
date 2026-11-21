// ViviaBreathe mobil – Gerätefunktionen für die App: Bildschirm anlassen, Vibration, Dateizugriff,
// Teilen, Einstellungen öffnen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.app.Activity;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.provider.OpenableColumns;
import android.view.WindowManager;

import androidx.core.content.FileProvider;

import java.io.File;
import java.util.ArrayList;

public class Device {
    // Während einer Übung den Bildschirm nicht ausschalten
    public static void keepScreenOn(Context ctx, final boolean on) {
        if (!(ctx instanceof Activity)) return;
        final Activity a = (Activity) ctx;
        a.runOnUiThread(() -> {
            if (on) a.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            else a.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        });
    }

    @SuppressWarnings("deprecation")      // VIBRATOR_SERVICE bis Android 11
    public static void vibrate(Context ctx, int ms) {
        try {
            Vibrator v;
            if (Build.VERSION.SDK_INT >= 31) {
                VibratorManager vm = (VibratorManager) ctx.getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
                v = vm.getDefaultVibrator();
            } else {
                v = (Vibrator) ctx.getSystemService(Context.VIBRATOR_SERVICE);
            }
            if (v == null || !v.hasVibrator()) return;
            v.vibrate(VibrationEffect.createOneShot(Math.max(20, ms), VibrationEffect.DEFAULT_AMPLITUDE));
        } catch (Exception e) { /* ohne Vibration */ }
    }

    // Leserecht einer in der Dateiauswahl gewählten Datei dauerhaft behalten (für Wecker und Musik)
    public static boolean persistUri(Context ctx, String uri) {
        try {
            ctx.getContentResolver().takePersistableUriPermission(Uri.parse(uri), Intent.FLAG_GRANT_READ_URI_PERMISSION);
            return true;
        } catch (Exception e) {
            return false;        // nicht dauerhaft freigebbar (z. B. Datei einer anderen App): gilt bis zum Neustart
        }
    }

    // lesbarer Name einer Datei (auch content://)
    public static String displayName(Context ctx, String f) {
        if (f == null || f.isEmpty()) return "";
        if (f.startsWith("content:")) {
            try (Cursor c = ctx.getContentResolver().query(Uri.parse(f), new String[] { OpenableColumns.DISPLAY_NAME }, null, null, null)) {
                if (c != null && c.moveToFirst()) {
                    String n = c.getString(0);
                    if (n != null && !n.isEmpty()) return n;
                }
            } catch (Exception e) { /* kein Zugriff mehr */ }
            String last = Uri.parse(f).getLastPathSegment();
            return last == null ? f : last;
        }
        int slash = f.lastIndexOf('/');
        return slash >= 0 ? f.substring(slash + 1) : f;
    }

    // Dateien über das Teilen-Menü von Android weitergeben (E-Mail, Messenger, Cloud …)
    public static boolean share(Context ctx, String pathsJoined, String mime, String subject, String chooserTitle) {
        try {
            ArrayList<Uri> uris = new ArrayList<>();
            for (String p : pathsJoined.split("\n")) {
                if (p.isEmpty()) continue;
                uris.add(FileProvider.getUriForFile(ctx, ctx.getPackageName() + ".qtprovider", new File(p)));
            }
            if (uris.isEmpty()) return false;
            Intent send;
            if (uris.size() == 1) {
                send = new Intent(Intent.ACTION_SEND);
                send.putExtra(Intent.EXTRA_STREAM, uris.get(0));
            } else {
                send = new Intent(Intent.ACTION_SEND_MULTIPLE);
                send.putParcelableArrayListExtra(Intent.EXTRA_STREAM, uris);
            }
            ClipData clip = ClipData.newRawUri("", uris.get(0));
            for (int k = 1; k < uris.size(); ++k) clip.addItem(new ClipData.Item(uris.get(k)));
            send.setClipData(clip);
            send.setType(mime);
            send.putExtra(Intent.EXTRA_SUBJECT, subject);
            send.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            Intent chooser = Intent.createChooser(send, chooserTitle);
            chooser.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_GRANT_READ_URI_PERMISSION);
            ctx.startActivity(chooser);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    // Benachrichtigungs-Einstellungen von ViviaBreathe öffnen
    public static void openNotificationSettings(Context ctx) {
        try {
            Intent i = new Intent(android.provider.Settings.ACTION_APP_NOTIFICATION_SETTINGS);
            i.putExtra(android.provider.Settings.EXTRA_APP_PACKAGE, ctx.getPackageName());
            if (!(ctx instanceof Activity)) i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(i);
        } catch (Exception e) { /* Einstellung nicht vorhanden */ }
    }
}
