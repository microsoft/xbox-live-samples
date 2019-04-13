// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android;

import xbl.sample.android.views.*;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity
{
    private static final String TAG = "AndroidSample";

    // Used to load the 'native-lib' library on application startup.
    static { System.loadLibrary("native-lib"); }

    private native void InitializeGame();
    private native void CleanupGame();

    public IdentityView identityView;
    public MenuView menuView;
    public ConsoleView consoleView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        identityView = new IdentityView(this);
        menuView = new MenuView(this);
        consoleView = new ConsoleView(this);

        InitializeGame();

        identityView.identityLayer.SignInUserSilently();
    }

    @Override
    protected void onDestroy()
    {
        CleanupGame();

        super.onDestroy();
    }

    public String GetLocalStoragePath()
    {
        return this.getFilesDir().getPath();
    }
}