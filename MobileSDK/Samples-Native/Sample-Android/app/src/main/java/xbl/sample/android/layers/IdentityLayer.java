// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android.layers;

import xbl.sample.android.MainActivity;
import xbl.sample.android.R;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.os.AsyncTask;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ImageView;
import java.io.InputStream;

public class IdentityLayer
{
    private static final String TAG = "Identity Layer";

    public native void InitializeNativeVars();
    public native void SignInUserSilently();
    public native void SignInUserWithUI();
    public native void SignOutUser();

    private MainActivity m_activity;

    private Button m_buttonSignIn;
    private Button m_buttonSignOut;
    private ImageView m_imageViewGamerPic;
    private TextView m_textViewGamerScore;
    private TextView m_textViewGamerTag;

    public IdentityLayer(MainActivity activity)
    {
        m_activity = activity;

        // Setup GamerPic ImageView
        {
            m_imageViewGamerPic = m_activity.findViewById(R.id.gamerPic);
        }

        // Setup GamerScore TextView
        {
            m_textViewGamerScore = m_activity.findViewById(R.id.gamerScore);
        }

        // Setup GamerTag TextView
        {
            m_textViewGamerTag = m_activity.findViewById(R.id.gamerTag);
        }

        // Setup Sign In Button
        {
            m_buttonSignIn = m_activity.findViewById(R.id.button_SignIn);
            m_buttonSignIn.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    SignInUserWithUI();
                }
            });

            m_buttonSignIn.setEnabled(false);
        }

        // Setup Sign Out Button
        {
            m_buttonSignOut = m_activity.findViewById(R.id.button_SignOut);
            m_buttonSignOut.setOnClickListener(new View.OnClickListener()
            {
                public void onClick(View v)
                {
                    SignOutUser();
                }
            });

            m_buttonSignOut.setEnabled(false);
        }

        InitializeNativeVars();
    }

    class DownloadImageTask extends AsyncTask<String, Void, Bitmap>
    {
        private ImageView m_imageView;

        public DownloadImageTask(ImageView bmImage)
        {
            m_imageView = bmImage;
        }

        protected Bitmap doInBackground(String... URL)
        {
            String url = URL[0];
            Bitmap bitmap = null;
            try
            {
                InputStream inStream = new java.net.URL(url).openStream();
                bitmap = BitmapFactory.decodeStream(inStream);
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }

            if (bitmap != null)
            {
                Bitmap roundedBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), bitmap.getConfig());
                Canvas canvas = new Canvas(roundedBitmap);

                BitmapShader shader = new BitmapShader(bitmap, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);

                Rect rect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
                RectF rectF = new RectF(rect);
                float roundPx = bitmap.getWidth()/2.0f;
                float roundPy = bitmap.getHeight()/2.0f;
                Paint paint = new Paint();
                paint.setAntiAlias(true);
                paint.setShader(shader);

                canvas.drawRoundRect(rectF, roundPx, roundPy, paint);

                bitmap = roundedBitmap;
            }

            return bitmap;
        }

        protected void onPostExecute(Bitmap result)
        {
            m_imageView.setImageBitmap(result);
        }
    }

    private class SetGamerPicRunnable implements Runnable
    {
        private String m_string;

        SetGamerPicRunnable(String gamerPic)
        {
            m_string = gamerPic;
        }

        @Override
        public void run()
        {
            new DownloadImageTask(m_imageViewGamerPic).execute(m_string);
        }
    }

    public void setGamerPic(String gamerPic)
    {
        SetGamerPicRunnable runnable = new SetGamerPicRunnable(gamerPic);
        m_activity.runOnUiThread(runnable);
    }

    private class ClearGamerPicRunnable implements Runnable
    {
        @Override
        public void run()
        {
            m_imageViewGamerPic.setImageResource(R.mipmap.ic_xboxlive_round);
        }
    }

    public void clearGamerPic()
    {
        ClearGamerPicRunnable runnable = new ClearGamerPicRunnable();
        m_activity.runOnUiThread(runnable);
    }

    private class SetGamerScoreRunnable implements Runnable
    {
        private String m_string;

        SetGamerScoreRunnable(String gamerScore)
        {
            m_string = gamerScore;
        }

        @Override
        public void run()
        {
            m_textViewGamerScore.setText(m_string);
        }
    }

    public void setGamerScore(String gamerScore)
    {
        SetGamerScoreRunnable runnable = new SetGamerScoreRunnable(gamerScore);
        m_activity.runOnUiThread(runnable);
    }

    private class ClearGamerScoreRunnable implements Runnable
    {
        @Override
        public void run()
        {
            m_textViewGamerScore.setText("");
        }
    }

    public void clearGamerScore()
    {
        ClearGamerScoreRunnable runnable = new ClearGamerScoreRunnable();
        m_activity.runOnUiThread(runnable);
    }


    private class SetGamerTagRunnable implements Runnable
    {
        private String m_string;

        SetGamerTagRunnable(String gamerTag)
        {
            m_string = gamerTag;
        }

        @Override
        public void run()
        {
            m_textViewGamerTag.setText(m_string);
        }
    }

    public void setGamerTag(String gamerTag)
    {
        SetGamerTagRunnable runnable = new SetGamerTagRunnable(gamerTag);
        m_activity.runOnUiThread(runnable);
    }

    private class ClearGamerTagRunnable implements Runnable
    {
        @Override
        public void run()
        {
            m_textViewGamerTag.setText(R.string.app_name);
        }
    }

    public void clearGamerTag()
    {
        ClearGamerTagRunnable runnable = new ClearGamerTagRunnable();
        m_activity.runOnUiThread(runnable);
    }

    private class SetSignInRunnable implements Runnable
    {
        private Boolean m_value;

        SetSignInRunnable(Boolean value)
        {
            m_value = value;
        }

        @Override
        public void run()
        {
            m_buttonSignIn.setEnabled(m_value);
        }
    }

    public void setSignInEnabled(boolean value)
    {
        SetSignInRunnable runnable = new SetSignInRunnable(value);
        m_activity.runOnUiThread(runnable);
    }

    private class SetSignOutRunnable implements Runnable
    {
        private Boolean m_value;

        SetSignOutRunnable(Boolean value)
        {
            m_value = value;
        }

        @Override
        public void run()
        {
            m_buttonSignOut.setEnabled(m_value);
        }
    }

    public void setSignOutEnabled(boolean value)
    {
        SetSignOutRunnable runnable = new SetSignOutRunnable(value);
        m_activity.runOnUiThread(runnable);
    }
}