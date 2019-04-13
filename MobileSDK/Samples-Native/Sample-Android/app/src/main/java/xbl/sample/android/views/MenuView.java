// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android.views;

import xbl.sample.android.MainActivity;
import xbl.sample.android.R;
import xbl.sample.android.layers.*;
import android.widget.LinearLayout;

public class MenuView
{
    private static final String TAG = "Menu View";

    public native void InitializeNativeVars();

    // Add Layers here
    public MainMenuLayer mainMenuLayer;
    public AchievementsLayer achievementsLayer;

    // Add Menu View Layer (MVL), make sure to add to MenuView_JNI as well!
    public static final int MVL_EMPTY = 0;
    public static final int MVL_MAIN_MENU = 1;
    public static final int MVL_ACHIEVEMENTS = 2;

    private MainActivity m_activity;
    private LinearLayout m_layout;

    public MenuView(MainActivity activity)
    {
        m_activity = activity;
        m_layout = activity.findViewById(R.id.menuLayout);

        // Allocate Layers
        mainMenuLayer = new MainMenuLayer(activity);
        achievementsLayer = new AchievementsLayer(activity);

        // Reset View, just encase
        resetView();

        InitializeNativeVars();
    }

    private void resetView()
    {
        if (m_layout == null) { return; }

        if (m_layout.getChildCount() > 0) { m_layout.removeAllViews(); }
    }

    private class ChangeLayerRunnable implements Runnable
    {
        private int m_layer;

        ChangeLayerRunnable(int layer)
        {
            m_layer = layer;
        }

        @Override
        public void run()
        {
            resetView();

            switch(m_layer)
            {
                case MVL_ACHIEVEMENTS:
                {
                    achievementsLayer.show(m_layout);
                }break;
                case MVL_MAIN_MENU:
                {
                    mainMenuLayer.show(m_layout);
                }break;
                case MVL_EMPTY:
                default:
                    break;
            }
        }
    }

    public void changeLayer(int layer)
    {
        ChangeLayerRunnable runnable = new ChangeLayerRunnable(layer);
        m_activity.runOnUiThread(runnable);
    }
}