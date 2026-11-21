// ViviaBreathe mobil – eine angetippte Benachrichtigung merken und die App nach vorn holen
//
// Die App holt die gemerkte Aktion beim Aktivwerden ab (take): Übung des Ereignisses öffnen,
// Einschlafen anbieten, laufenden Wecker zeigen.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;

import org.json.JSONObject;

public class OpenActivity extends Activity {
    static final String PREFS = "viviabreathe_launch";

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        Intent in = getIntent();
        if (in != null && in.hasExtra("kind")) {
            try {
                JSONObject o = new JSONObject();
                o.put("kind", in.getStringExtra("kind"));
                o.put("ref", in.getLongExtra("ref", 0));
                o.put("user", in.getLongExtra("user", 0));
                o.put("at", System.currentTimeMillis());
                getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString("action", o.toString()).apply();
            } catch (Exception e) { /* ohne Aktion einfach öffnen */ }
        }
        Intent launch = getPackageManager().getLaunchIntentForPackage(getPackageName());
        if (launch != null) {
            launch.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            startActivity(launch);
        }
        finish();
    }

    // Gemerkte Aktion abholen (einmalig; älter als 6 Stunden verfällt sie)
    public static String take(Context ctx) {
        SharedPreferences p = ctx.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        String s = p.getString("action", "");
        if (s.isEmpty()) return "";
        p.edit().remove("action").apply();
        try {
            JSONObject o = new JSONObject(s);
            if (System.currentTimeMillis() - o.optLong("at", 0) > 6L * 3600 * 1000) return "";
        } catch (Exception e) { return ""; }
        return s;
    }

    static Intent intentFor(Context ctx, String kind, long ref, long user) {
        Intent i = new Intent(ctx, OpenActivity.class);
        i.setAction("de.mbergmann.viviabreathe.OPEN." + kind + "." + ref);
        i.putExtra("kind", kind);
        i.putExtra("ref", ref);
        i.putExtra("user", user);
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return i;
    }
}
