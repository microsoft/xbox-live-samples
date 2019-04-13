// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package xbl.sample.android.views;

import xbl.sample.android.MainActivity;
import xbl.sample.android.layers.IdentityLayer;

public class IdentityView
{
    private static final String TAG = "Identity View";

    public IdentityLayer identityLayer;

    public IdentityView(MainActivity activity)
    {
        identityLayer = new IdentityLayer(activity);
    }
}