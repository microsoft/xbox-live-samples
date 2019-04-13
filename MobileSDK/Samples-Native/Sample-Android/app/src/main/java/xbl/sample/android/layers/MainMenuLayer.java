// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android.layers;

import xbl.sample.android.MainActivity;
import xbl.sample.android.R;
import xbl.sample.android.views.MenuView;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.Button;
import android.graphics.Color;

public class MainMenuLayer
{
    private static final String TAG = "Main Menu Layer";

    private MainActivity m_activity;

    private Button m_buttonAchievements;

    public MainMenuLayer(MainActivity activity)
    {
        m_activity = activity;

        // Create Achievements Button
        {
            m_buttonAchievements = new Button(m_activity);
            m_buttonAchievements.setText(R.string.button_achievements);
            m_buttonAchievements.setTextColor(Color.BLACK);
            m_buttonAchievements.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonAchievements.setHeight(48);
            m_buttonAchievements.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    m_activity.menuView.changeLayer(MenuView.MVL_ACHIEVEMENTS);
                }
            });
        }
    }

    public void show(LinearLayout layout)
    {
        layout.addView(m_buttonAchievements);
    }
}