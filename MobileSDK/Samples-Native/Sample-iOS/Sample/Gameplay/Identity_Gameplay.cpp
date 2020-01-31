// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <Identity_Integration.h>
#import <XSAPI_Integration.h>
#import <Game_Integration.h>
#import <HubMenu_Integration.h>
#import <IdentityMenu_Integration.h>
#import <IdentityDisplayMenu_Integration.h>

#pragma region Identity Gameplay Internal

void Identity_Gameplay_CloseUserContext(_In_ XblContextHandle xblContext)
{
    if (xblContext)
    {
        XblContextCloseHandle(xblContext);
    }
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
            Identity_Gameplay_CloseUserContext(Game_Integration::getInstance()->getXblContext());
            Game_Integration::getInstance()->setXalUser(newUser);
            Game_Integration::getInstance()->setXblContext(newXblContext);
            
            SampleLog(LL_INFO, ""); // New line
            if (resolveIssuesWithUI)
            {
                SampleLog(LL_INFO, "Sign-in with UI successful!");
            }
            else
            {
                SampleLog(LL_INFO, "Auto sign-in successful!");
            }
            
            std::string gamerTag;
            hr = Identity_GetGamerTag(newUser, &gamerTag);
            if (SUCCEEDED(hr))
            {
                SampleLog(LL_INFO, "Welcome %s!", gamerTag.c_str());
                IdentityMenu_Integration::getInstance()->updateIdentityTitle(gamerTag.c_str());
            }

            IdentityMenu_Integration::getInstance()->updateIdentityButtons(ID_SIGNED_IN);
            IdentityMenu_Integration::getInstance()->updateIdentityContext(ID_SIGNED_IN);
            HubMenu_Integration::getInstance()->setHubMenuHidden(false);
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
            SampleLog(LL_INFO, ""); // New Line
            SampleLog(LL_INFO, "Trying to resolve user issue with UI");
 
            HRESULT asyncResult = Identity_TryResolveUserIssue(XblGetAsyncQueue(), newUser);
            
            if (FAILED(asyncResult))
            {
                SampleLog(LL_ERROR, "XalUserResolveIssueWithUiAsync Failed!");
                SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(asyncResult).c_str());
            }
        }
    }
    
    return hr;
}

#pragma endregion

#pragma region Identity Gameplay

void Identity_Gameplay_TrySignInUserSilently(
    _In_ XalUserHandle newUser,
    _In_ HRESULT result)
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
        
        IdentityMenu_Integration::getInstance()->updateIdentityButtons(ID_SIGNED_OUT);
        HubMenu_Integration::getInstance()->setHubMenuHidden(true);
    }
}

void Identity_Gameplay_TrySignInUserWithUI(
    _In_ XalUserHandle newUser,
    _In_ HRESULT result)
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

        IdentityMenu_Integration::getInstance()->updateIdentityButtons(ID_SIGNED_OUT);
        HubMenu_Integration::getInstance()->setHubMenuHidden(true);
    }
}

void Identity_Gameplay_TryResolveUserIssue(
    _In_ XalUserHandle user,
    _In_ HRESULT result)
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

    SampleLog(LL_INFO, ""); // New Line
    SampleLog(LL_INFO, "Please try signing-in with UI again!");

    // Close the Reference if one was created during XalUserDuplicateHandle
    if (user) { XalUserCloseHandle(user); }
}

void Identity_Gameplay_TrySignOutUser(
    _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();

        XalUserHandle user = Game_Integration::getInstance()->getCurrentUser();
        std::string gamerTag;
        auto hr = Identity_GetGamerTag(user, &gamerTag);
        if (SUCCEEDED(hr))
        {
            SampleLog(LL_INFO, "Goodbye %s!", gamerTag.c_str());
            IdentityMenu_Integration::getInstance()->updateIdentityTitle(nullptr);
            IdentityMenu_Integration::getInstance()->updateIdentityGamerScore(nullptr);
            IdentityMenu_Integration::getInstance()->updateIdentityImage(nullptr);
        }
        
        IdentityMenu_Integration::getInstance()->updateIdentityButtons(ID_SIGNED_OUT);
        IdentityMenu_Integration::getInstance()->updateIdentityContext(ID_SIGNED_OUT);
        Identity_Gameplay_CloseUserContext(xblContext);
        Game_Integration::getInstance()->setXblContext(nullptr);
        HubMenu_Integration::getInstance()->setHubMenuHidden(true);
        XalUserCloseHandle(user);
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "User sign-out successful!");
    }
    else
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "User sign-out failed!");
    }
}

void Identity_Gameplay_GetDefaultGamerProfile(
    _In_ XblUserProfile userProfile,
    _In_ HRESULT result)
{
    HRESULT hr = result;
    
    if (SUCCEEDED(hr))
    {
        IdentityMenu_Integration::getInstance()->updateIdentityImage(userProfile.gameDisplayPictureResizeUri);
        IdentityMenu_Integration::getInstance()->updateIdentityTitle(userProfile.gamertag);
        IdentityMenu_Integration::getInstance()->updateIdentityGamerScore(userProfile.gamerscore);
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, "Get Gamer Profile failed!");
    }
}

void Identity_Gameplay_GetGamerProfile(
    _In_ XblUserProfile userProfile,
    _In_ HRESULT result)
{
    HRESULT hr = result;
    
    if (SUCCEEDED(hr))
    {
        IdentityDisplayMenu_Integration::getInstance()->updateUserImage(userProfile.gameDisplayPictureResizeUri);
        IdentityDisplayMenu_Integration::getInstance()->updateUserTitle(userProfile.gamertag);
        IdentityDisplayMenu_Integration::getInstance()->updateUserGamerScore(userProfile.gamerscore);
        //SocialUserMenu_Integration::getInstance()->updateUserStatus(userProfile.status);
        //SocialUserMenu_Integration::getInstance()->updateUserRelationship(userProfile.relationship);
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, "Get Gamer Profile failed!");
    }
}

#pragma endregion
