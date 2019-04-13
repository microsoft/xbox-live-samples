// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android.views;

import xbl.sample.android.MainActivity;
import xbl.sample.android.R;
import android.view.View;
import android.widget.ScrollView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.graphics.Color;

public class ConsoleView
{
    private static final String TAG = "Console View";

    private native void InitializeConsole();

    private MainActivity m_activity;
    private ScrollView m_scrollView;
    private LinearLayout m_consoleLayout;

    public static final int LL_FATAL   = 0x01;
    public static final int LL_ERROR   = 0x02;
    public static final int LL_WARNING = 0x04;
    public static final int LL_INFO    = 0x08;
    public static final int LL_DEBUG   = 0x10;
    public static final int LL_TRACE   = 0x20;

    public ConsoleView(MainActivity activity)
    {
        m_activity = activity;

        m_scrollView = m_activity.findViewById(R.id.consoleScrollView);
        m_consoleLayout = m_activity.findViewById(R.id.consoleLayout);

        InitializeConsole();
    }

    private class LogRunnable implements Runnable
    {
        private int m_logLevel;
        private String m_string;

        LogRunnable(int logLevel, String logString)
        {
            m_logLevel = logLevel;
            m_string = logString;
        }

        @Override
        public void run()
        {
            TextView textView = new TextView(m_activity);

            switch(m_logLevel)
            {
                case LL_WARNING:
                {
                    textView.setTextColor(Color.YELLOW);
                }break;
                case LL_INFO:
                {
                    textView.setTextColor(Color.WHITE);
                }break;
                case LL_DEBUG:
                {
                    textView.setTextColor(Color.GREEN);
                }break;
                case LL_TRACE:
                {
                    textView.setTextColor(Color.GRAY);
                }break;
                case LL_FATAL:
                case LL_ERROR:
                default:
                {
                    textView.setTextColor(Color.RED);
                }
            }

            textView.setTextSize(18.0f);
            textView.setText(m_string);

            m_consoleLayout.addView(textView);

            m_scrollView.post(new Runnable()
            {
                @Override
                public void run()
                {
                    m_scrollView.fullScroll(View.FOCUS_DOWN);
                }
            });
        }
    }

    public void log(int logLevel, String string)
    {
        LogRunnable runnable = new LogRunnable(logLevel, string);
        m_activity.runOnUiThread(runnable);
    }
}