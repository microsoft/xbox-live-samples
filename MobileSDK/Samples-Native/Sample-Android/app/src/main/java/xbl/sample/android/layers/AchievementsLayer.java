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

public class AchievementsLayer
{
    private static final String TAG = "Achievements Layer";

    public native void InitializeNativeVars();
    public native void GetAchievements();
    public native void GetNextPage();
    public native void GetAchievement();
    public native void UpdateAchievement();

    private MainActivity m_activity;

    private Button m_buttonGetAchievements;
    private Button m_buttonGetNextPage;
    private Button m_buttonGetAchievement;
    private Button m_buttonUpdateAchievement;
    private Button m_buttonMainMenu;

    public AchievementsLayer(MainActivity activity)
    {
        m_activity = activity;

        // Create Get Achievements Button
        {
            m_buttonGetAchievements = new Button(m_activity);
            m_buttonGetAchievements.setText(R.string.button_getAchievements);
            m_buttonGetAchievements.setTextColor(Color.BLACK);
            m_buttonGetAchievements.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonGetAchievements.setHeight(48);
            m_buttonGetAchievements.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    GetAchievements();
                }
            });
        }

        // Create Get Next Page Button
        {
            m_buttonGetNextPage = new Button(m_activity);
            m_buttonGetNextPage.setText(R.string.button_getNextPage);
            m_buttonGetNextPage.setTextColor(Color.BLACK);
            m_buttonGetNextPage.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonGetNextPage.setHeight(48);
            m_buttonGetNextPage.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    GetNextPage();
                }
            });

            setHasNextPage(false);
        }

        // Create Get Achievement Button
        {
            m_buttonGetAchievement = new Button(m_activity);
            m_buttonGetAchievement.setText(R.string.button_getAchievement);
            m_buttonGetAchievement.setTextColor(Color.BLACK);
            m_buttonGetAchievement.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonGetAchievement.setHeight(48);
            m_buttonGetAchievement.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    GetAchievement();
                }
            });
        }

        // Create Update Achievement Button
        {
            m_buttonUpdateAchievement = new Button(m_activity);
            m_buttonUpdateAchievement.setText(R.string.button_updateAchievement);
            m_buttonUpdateAchievement.setTextColor(Color.BLACK);
            m_buttonUpdateAchievement.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonUpdateAchievement.setHeight(48);
            m_buttonUpdateAchievement.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    UpdateAchievement();
                }
            });
        }

        // Create Menu Button
        {
            m_buttonMainMenu = new Button(m_activity);
            m_buttonMainMenu.setText(R.string.button_mainMenu);
            m_buttonMainMenu.setTextColor(Color.BLACK);
            m_buttonMainMenu.setBackgroundResource(android.R.drawable.btn_default);
            m_buttonMainMenu.setHeight(48);
            m_buttonMainMenu.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    m_activity.menuView.changeLayer(MenuView.MVL_MAIN_MENU);
                }
            });
        }

        InitializeNativeVars();
    }

    public void show(LinearLayout layout)
    {
        layout.addView(m_buttonGetAchievements);
        layout.addView(m_buttonGetNextPage);
        layout.addView(m_buttonGetAchievement);
        layout.addView(m_buttonUpdateAchievement);
        layout.addView(m_buttonMainMenu);
    }

    private class SetHasNextPageRunnable implements Runnable
    {
        private boolean m_value;

        SetHasNextPageRunnable(boolean value)
        {
            m_value = value;
        }

        @Override
        public void run()
        {
            m_buttonGetNextPage.setEnabled(m_value);
        }
    }

    public void setHasNextPage(boolean value)
    {
        SetHasNextPageRunnable runnable = new SetHasNextPageRunnable(value);
        m_activity.runOnUiThread(runnable);
    }
}