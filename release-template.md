# Ace Combat 7 UEVR compatibility mod

## Mod Features
1. **Fixes cockpit camera** so it works out of the box in most situations
1. Adds **'unstuck cockpit instruments' button** (left start/select button on Xbox controller) for those rare occurrences where automation fails. 

	**Button 3** on the image below: 
	![](https://compass-ssl.xboxlive.com/assets/4a/5a/4a5a376a-50b3-444c-813d-248d4f82b51c.png?n=SXC-Article-MerlinControllerBackTopCallouts-L-16x9794x445-01.png)
1. Adds **optional controls remap** selectable in plugin overlay in UEVR menu. For remap any standard RC control mode can be selected: ![RC control modes](https://i.stack.imgur.com/3O98c.png)
1. works **only with VR plane skins added by Konan's VR Planes mod** (see below). **You need to beat campaign at least once** on any difficulty for skin slots to unlock. Otherwise there's no way to select VR version of a plane. There's a quick way to cheat trough all missions [here](https://discord.com/channels/747967102895390741/1071072263820021840/1219319580195819621)

## Note
* This is an alternative for [kosnag's UEVR compatibility mod](https://discord.com/channels/747967102895390741/1071072263820021840/1200018686317187092) (`~~~UEVR_Compatibility_Mod_P.pak`). **Do NOT install both** at the same time. The game will be very broken if you do.

## Status/known issues
**The whole game has been tested** and can be beaten using this mod. 

Quirks:
* in takeoff sequences where plane is stationary throttle up as soon as you can and let the plane move to fix the camera. This is a tradeoff for camera working well in so many other places.
* if you retry checkpoint you have to push 'unstuck cockpit instruments' button after you have control over the plane. So far this worked perfectly for me unlike with OG mod.
* otherwise it just works. If [kosnag's UEVR compatibility mod](https://discord.com/channels/747967102895390741/1071072263820021840/1200018686317187092) just worked 95% of the time, this works 99% with 1% being stated above.

## Getting started

### Joining Flat2VR discord

Flat2VR channel list is tiered. You need to tell the server that you want to see UEVR related stuff (channels with `#ue-` prefix).
1. accept [Flat2VR Discord](https://discord.gg/67RFQscp) invite
1. go to `#ue-join` and enable UEVR channels visibility (button below the message)
1. go to [Ace Combat 7 channel](https://discord.com/channels/747967102895390741/1071072263820021840) in `#ue-games`.

### Installing prerequisite mods
1. after joining the Flat2VR discord install [Konan's VR Planes mod](https://discord.com/channels/747967102895390741/1071072263820021840/1216021454563446835). Follow the instructions in `message.txt` file closely. Konan's `AC7_VR_PatchP.pak` from the same link is required as well. **Do not install** kosnag's uevr compatibility mod (`~~~UEVR_Compatibility_Mod_P.pak`). This profile replaces it.
1. in short you need:
	* [My special version](https://discord.com/channels/747967102895390741/1071072263820021840/1218606543994486806) of `Playable_VR_Planes_for_test_only_P.pak` to fix some planes with quirks. Rename to `~~~~~~~Playable VR Planes (for test only)_P.pak`
	* `AC7_VR_Patch_P.pak` from [Konan's VR Planes mod](https://discord.com/channels/747967102895390741/1071072263820021840/1216021454563446835). Rename to `~~~~~~~AC7 VR Patch_P.pak`
	* `Additional_Skin_Slots_P.pak` from [Additional Skin Slots mod](https://www.nexusmods.com/acecombat7skiesunknown/mods/2179)
	* `~~~~~~~~~!since_SDT_Add-On-ASS_P.pak` from `since_SDT_Add-On_P.pak` download option in [sincerity's essential files](https://www.nexusmods.com/acecombat7skiesunknown/mods/2274)
1. [this is how your ~mods folder should look like](https://cdn.discordapp.com/attachments/1071072263820021840/1216517542667878440/image.png?ex=6600ad3f&is=65ee383f&hm=e09af0ea62b131f90924a7b526ff11ffd12d2782bac72f808f599de842bbaf77&). If in doubt use [Konan's VR Planes mod](https://discord.com/channels/747967102895390741/1071072263820021840/1216021454563446835) install instructions over the picture.

### [UEVR Nightly](https://github.com/praydog/UEVR-nightly/releases/tag/nightly-846-3b206447ac202795afd99c0e5391cac28823920c) `3b206447ac202795afd99c0e5391cac28823920c` or newer is required

### Installing the mod
1. download `Ace7Game.zip` from [here](https://github.com/keton/ace-combat-uevr/releases/latest/download/Ace7Game.zip)
1. import it as profile into UEVR using dedicated button in UEVR injector or unpack to `%APPDATA%\UnrealVRMod\Ace7Game` (you'll need to create `Ace7Game` or empty it before unpacking)

### Running
1. put on your HMD
1. launch AC7 as a flat game
1. stop at the main menu
1. inject using UEVR nightly from above (optionally with mrbelowski's patch)
1. (for first time verification only) press and hold L3+R3 till UEVR overlay pops up. It's set to long press so flares work in game. Look for blue overlay panel labelled `Ace Combat Plugin`. If you see it plugin is loaded. If not something went wrong.
1. go into a mission, pick a plane
1. **important** in plane customization screen go into skin selection menu and go to the very bottom. It should have 10 extra skin slots and you want the last one. For that you have to have **completed campaign at least once** on any difficulty. Without that 'select skin' option won't show in game. If you don't see 10 extra skin slots under standard skins in menu Additional Skin Slots mod is not installed correctly. Go trough the steps again or ask on Discord.
1. **you really need to select 10th additional skin slot**. This is what triggers Konan's VR Planes mod. Without it displays in the cockpit and HUD won't work. Not all planes are already converted. Check documentation.
1. start mission as usual. Multi Function Displays (MFDs) should work. In case they get stuck press 'Unstuck cockpit button' - Xbox controller 'back' button (left small round one opposite to menu button)
1. Camera breaks in some missions/planes? 'Unstuck cockpit button' fails to work? Please help by reporting on Discord.

## Acknowledgements
* Credit goes to kosnag, the author of [original UEVR compatibility mod](https://discord.com/channels/747967102895390741/1071072263820021840/1200018686317187092) for discovering 'shake camera for 3d' fix. This required huge amount of time spent on research and hacking the game. And it made this mod possible. Hats off.
* Another round of credit and appreciation goes to Konan, the author of [VR Planes mod](https://discord.com/channels/747967102895390741/1071072263820021840/1216021454563446835). The mod required meticulous hand crafting each plane to accommodate special VR version of the instruments.
