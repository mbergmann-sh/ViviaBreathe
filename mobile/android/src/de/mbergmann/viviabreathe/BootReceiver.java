// ViviaBreathe mobil – nach einem Neustart (oder Update) die gestellten Erinnerungen und Wecker wiederherstellen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
package de.mbergmann.viviabreathe;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class BootReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context ctx, Intent intent) {
        Reminders.restore(ctx);
    }
}
