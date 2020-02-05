// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include <Identity_Integration.h>
#include <Game_Integration.h>
#include <IdentityLayer_JNI.h>
#include <MenuView_JNI.h>

#pragma region Identity Gameplay Internal
void Identity_Gameplay_UserSignInOutMessage(XalUserHandle user, bool isSigningIn)
{
    if (user)
    {
        char gamerTag[XBL_GAMERTAG_CHAR_SIZE] = {0};
        size_t gamertagSize = 0;

        HRESULT hr = XalUserGetGamertag(user, XalGamertagComponent::XalGamertagComponent_Classic, XBL_GAMERTAG_CHAR_SIZE, gamerTag, &gamertagSize);

        if (SUCCEEDED(hr))
        {
            SampleLog(LL_INFO, ""); // New Line
            if (isSigningIn)
            {
                SampleLog(LL_INFO, "Welcome %s!", gamerTag);
            }
            else
            {
                SampleLog(LL_INFO, "Goodbye %s!", gamerTag);
            }
        }
        else
        {
            SampleLog(LL_ERROR, "XalUserGetGamertag failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }
}

void Identity_Gameplay_WelcomeUser(XalUserHandle user)
{
    Identity_Gameplay_UserSignInOutMessage(user, true);
}

void Identity_Gameplay_GoodbyeUser(XalUserHandle user)
{
    Identity_Gameplay_UserSignInOutMessage(user, false);
}

void Identity_Gameplay_CloseUserContext(_In_ XblContextHandle xblContext)
{
    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(xblContext, &user);

    if (SUCCEEDED(hr))
    {
        XalUserCloseHandle(user);
    }

    XblContextCloseHandle(xblContext);
}

HRESULT Identity_Gameplay_SignInUser(_In_ XalUserHandle newUser, _In_ bool resolveIssuesWithUI)
{
    // Call XalUserGetId here to ensure all vetos (gametag banned, etc) have passed
    uint64_t xuid = 0;
    HRESULT hr = XalUserGetId(newUser, &xuid);

    if (SUCCEEDED(hr))
    {
        XblContextHandle newXblContext = nullptr;
        hr = XblContextCreateHandle(newUser, &newXblContext);

        if (SUCCEEDED(hr))
        {
            // Close the previous Xbl Context, if one existed
            XblContextHandle oldXblContext = Game_Integration::getInstance()->getXblContext();

            if (oldXblContext != nullptr)
            {
                Identity_Gameplay_CloseUserContext(oldXblContext);
            }

            Game_Integration::getInstance()->setXblContext(newXblContext);

            SampleLog(LL_TRACE, ""); // New line
            if (resolveIssuesWithUI)
            {
                SampleLog(LL_TRACE, "Sign-in with UI successful!");
            }
            else
            {
                SampleLog(LL_TRACE, "Auto sign-in successful!");
            }

            Identity_Gameplay_WelcomeUser(newUser);

            Identity_GetDefaultGamerProfileAsync(XblGetAsyncQueue(), newXblContext);
        }
        else
        {
            SampleLog(LL_ERROR, "XblContextCreateHandle Failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }
    else
    {
        SampleLog(LL_ERROR, "XalUserGetId Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());

        if (resolveIssuesWithUI)
        {
            SampleLog(LL_TRACE, ""); // New Line
            SampleLog(LL_TRACE, "Trying to resolve user issue with UI");

            // Duplicate handle to prolong the user to be handled later by resolve
            XblUserHandle dupUser = nullptr;
            XalUserDuplicateHandle(newUser, &dupUser);
            // Note: Creates a Ref for XblUserHandle, will be closed inside XAL_Gameplay_TryResolveUserIssue

            HRESULT asyncResult = Identity_TryResolveUserIssue(XblGetAsyncQueue(), dupUser);

            if (FAILED(asyncResult))
            {
                SampleLog(LL_ERROR, "XalUserResolveIssueWithUiAsync Failed!");
                SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(asyncResult).c_str());

                if (dupUser) { XalUserCloseHandle(dupUser); }
            }
        }
    }

    return hr;
}

HRESULT Identity_Gameplay_SignOutUser()
{
    XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();

    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(xblContext, &user);

    if (SUCCEEDED(hr))
    {
        Identity_Gameplay_GoodbyeUser(user);

        IdentityLayer_ClearGamerPic();
        IdentityLayer_ClearGamerTag();
        IdentityLayer_ClearGamerScore();
        IdentityLayer_SetSignInEnabled(true);
        IdentityLayer_SetSignOutEnabled(false);

        MenuView_ChangeLayer(MVL_EMPTY);

        SampleLog(LL_TRACE, ""); // New line
        SampleLog(LL_TRACE, "User sign-out successful!");
    }

    if (xblContext)
    {
        Identity_Gameplay_CloseUserContext(xblContext);
        Game_Integration::getInstance()->setXblContext(nullptr);
    }

    return hr;
}
#pragma endregion

#pragma region Identity Gameplay
void Identity_Gameplay_TrySignInUserSilently(_In_ XalUserHandle newUser, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game
    HRESULT hr = result;

    if (SUCCEEDED(hr))
    {
        hr = Identity_Gameplay_SignInUser(newUser, false);
    }

    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "Auto sign-in failed! Please sign-in with the UI!");

        IdentityLayer_SetSignInEnabled(true);
    }
}

void Identity_Gameplay_TrySignInUserWithUI(_In_ XalUserHandle newUser, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game
    HRESULT hr = result;

    if (SUCCEEDED(hr))
    {
        hr = Identity_Gameplay_SignInUser(newUser, true);
    }

    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New Line
        SampleLog(LL_INFO, "Please try signing-in with UI again!");
    }
}

void Identity_Gameplay_TryResolveUserIssue(_In_ XalUserHandle user, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        SampleLog(LL_INFO, "Issues with sign-in resolved!");
    }
    else
    {
        SampleLog(LL_ERROR, "Issues with sign-in failed to resolve!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(result).c_str());
    }

    // Close the Reference if one was created during XalUserDuplicateHandle
    if (user) { XalUserCloseHandle(user); }
}

void Identity_Gameplay_TrySignOutUser(_In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game
    HRESULT hr = result;

    if (SUCCEEDED(hr))
    {
        hr = Identity_Gameplay_SignOutUser();
    }

    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "User sign-out failed!");
    }
}

void Identity_Gameplay_GetDefaultGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result)
{
    HRESULT hr = result;

    if (SUCCEEDED(hr))
    {
        IdentityLayer_SetGamerPic(userProfile.gameDisplayPictureResizeUri);
        IdentityLayer_SetGamerTag(userProfile.gamertag);
        IdentityLayer_SetGamerScore(userProfile.gamerscore);
        IdentityLayer_SetSignInEnabled(false);
        IdentityLayer_SetSignOutEnabled(true);

        MenuView_ChangeLayer(MVL_MAIN_MENU);
    }

    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "Getting Default Gamer Profile failed!");
    }
}

void Identity_Gameplay_GetGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result)
{
    HRESULT hr = result;

    if (SUCCEEDED(hr))
    {
        // TODO: Add in functionality for Social Module
    }

    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "Getting Gamer Profile failed!");
    }
}
#pragma endregion