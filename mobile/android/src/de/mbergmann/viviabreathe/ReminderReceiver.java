// ViviaBreathe mobil – ein gestellter Termin ist fällig: Benachrichtigung zeigen bzw. Musik starten
// (Wecker, automatisches Einschlafen)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class ReminderReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context ctx, Intent intent) {
        String play = intent.getStringExtra("play");
        if (play != null && !play.isEmpty()) {
            if (PlaybackService.startFromAlarm(ctx, play)) return;
            // Dienst ließ sich nicht starten (z. B. ungefährer Wecker): wenigstens benachrichtigen
        }
        Reminders.show(ctx, intent.getIntExtra("id", 0), intent.getStringExtra("kind"), intent.getLongExtra("ref", 0),
                       intent.getLongExtra("user", 0), intent.getStringExtra("title"), intent.getStringExtra("text"),
                       intent.getStringExtra("publicText"), intent.getBooleanExtra("sound", true));
    }
}
