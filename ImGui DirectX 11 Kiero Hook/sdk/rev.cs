using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using BattlEye;
using SDG.Framework.Modules;
using SDG.HostBans;
using SDG.NetPak;
using SDG.NetTransport;
using SDG.NetTransport.Loopback;
using SDG.Provider;
using SDG.Provider.Services.Multiplayer;
using SDG.SteamworksProvider;
using Steamworks;
using UnityEngine;
using UnityEngine.Networking;
using Unturned.SystemEx;
using Unturned.UnityEx;

namespace SDG.Unturned
{
	// Token: 0x02000663 RID: 1635
	public class Provider : MonoBehaviour
	{
		// Token: 0x17000978 RID: 2424
		// (get) Token: 0x0600362B RID: 13867 RVA: 0x0010A993 File Offset: 0x00108B93
		// (set) Token: 0x0600362C RID: 13868 RVA: 0x0010A99A File Offset: 0x00108B9A
		public static string APP_VERSION { get; protected set; }

		/// <summary>
		/// App version string packed into a 32-bit number for replication.
		/// </summary>
		// Token: 0x17000979 RID: 2425
		// (get) Token: 0x0600362D RID: 13869 RVA: 0x0010A9A2 File Offset: 0x00108BA2
		// (set) Token: 0x0600362E RID: 13870 RVA: 0x0010A9A9 File Offset: 0x00108BA9
		public static uint APP_VERSION_PACKED { get; protected set; }

		// Token: 0x0600362F RID: 13871 RVA: 0x0010A9B1 File Offset: 0x00108BB1
		private IEnumerator CaptureScreenshot()
		{
			bool enableScreenshotSupersampling = OptionsSettings.enableScreenshotSupersampling;
			int max = enableScreenshotSupersampling ? 4 : 16;
			int sizeMultiplier = Mathf.Clamp(OptionsSettings.screenshotSizeMultiplier, 1, max);
			int finalWidth = Screen.width * sizeMultiplier;
			int finalHeight = Screen.height * sizeMultiplier;
			int maxTextureSize = SystemInfo.maxTextureSize;
			if (finalWidth > maxTextureSize || finalHeight > maxTextureSize)
			{
				UnturnedLog.warn(string.Format("Unable to capture {0}x{1} screenshot because it exceeds max supported texture size ({2})", finalWidth, finalHeight, maxTextureSize));
				Provider.isCapturingScreenshot = false;
				yield break;
			}
			if (sizeMultiplier > 1 || enableScreenshotSupersampling)
			{
				UnturnedPostProcess.instance.DisableAntiAliasingForScreenshot = true;
			}
			string text = PathEx.Join(UnturnedPaths.RootDirectory, "Screenshots");
			Directory.CreateDirectory(text);
			bool flag;
			if (Level.isEditor && EditorUI.window != null)
			{
				flag = EditorUI.window.isEnabled;
			}
			else
			{
				flag = (!(Player.player != null) || PlayerUI.window == null || PlayerUI.window.isEnabled);
			}
			string str = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");
			if (!flag)
			{
				str += "_NoUI";
			}
			string filePath = Path.Combine(text, str + ".png");
			UnturnedLog.info(string.Format("Capturing {0}x{1} screenshot (Size Multiplier: {2} Use Supersampling: {3} HUD Visible: {4})", new object[]
			{
				finalWidth,
				finalHeight,
				sizeMultiplier,
				enableScreenshotSupersampling,
				flag
			}));
			if (!enableScreenshotSupersampling)
			{
				ScreenCapture.CaptureScreenshot(filePath, sizeMultiplier);
				yield return null;
				UnturnedPostProcess.instance.DisableAntiAliasingForScreenshot = false;
				float timePassed = 0f;
				for (;;)
				{
					timePassed += Time.deltaTime;
					if (File.Exists(filePath))
					{
						goto IL_45D;
					}
					if (timePassed >= 10f)
					{
						break;
					}
					yield return null;
				}
				UnturnedLog.error(string.Format("Screenshot file is not available after {0}s ({1})", timePassed, filePath));
				Provider.isCapturingScreenshot = false;
				yield break;
			}
			yield return new WaitForEndOfFrame();
			int superSize = sizeMultiplier * 2;
			Texture2D supersampledTexture = ScreenCapture.CaptureScreenshotAsTexture(superSize);
			UnturnedPostProcess.instance.DisableAntiAliasingForScreenshot = false;
			if (supersampledTexture == null)
			{
				UnturnedLog.error("CaptureScreenshotAsTexture returned null");
				Provider.isCapturingScreenshot = false;
				yield break;
			}
			yield return null;
			supersampledTexture.filterMode = FilterMode.Bilinear;
			RenderTexture downsampleRenderTexture = RenderTexture.GetTemporary(finalWidth, finalHeight, 0, supersampledTexture.graphicsFormat);
			Graphics.Blit(supersampledTexture, downsampleRenderTexture, Provider.screenshotBlitMaterial);
			yield return null;
			Texture2D downsampledTexture = new Texture2D(finalWidth, finalHeight, supersampledTexture.format, false, false);
			RenderTexture.active = downsampleRenderTexture;
			downsampledTexture.ReadPixels(new Rect(0f, 0f, (float)finalWidth, (float)finalHeight), 0, 0, false);
			RenderTexture.active = null;
			RenderTexture.ReleaseTemporary(downsampleRenderTexture);
			UnityEngine.Object.Destroy(supersampledTexture);
			yield return null;
			byte[] downsampledBytes = downsampledTexture.EncodeToPNG();
			UnityEngine.Object.Destroy(downsampledTexture);
			yield return null;
			File.WriteAllBytes(filePath, downsampledBytes);
			yield return null;
			supersampledTexture = null;
			downsampleRenderTexture = null;
			downsampledTexture = null;
			downsampledBytes = null;
			IL_45D:
			UnturnedLog.info("Captured screenshot: " + filePath);
			ScreenshotHandle hScreenshot = SteamScreenshots.AddScreenshotToLibrary(filePath, null, finalWidth, finalHeight);
			if (Level.info != null)
			{
				string localizedName = Level.info.getLocalizedName();
				SteamScreenshots.SetLocation(hScreenshot, localizedName);
				UnturnedLog.info("Tagged location \"" + localizedName + "\" in screenshot");
			}
			Camera instance = MainCamera.instance;
			if (instance != null)
			{
				Vector3 position = instance.transform.position;
				foreach (SteamPlayer steamPlayer in Provider.clients)
				{
					if (!(steamPlayer.player == null) && !steamPlayer.player.channel.IsLocalPlayer)
					{
						Vector3 vector = steamPlayer.player.transform.position + Vector3.up;
						if ((vector - position).sqrMagnitude <= 4096f)
						{
							Vector3 vector2 = instance.WorldToViewportPoint(vector);
							if (vector2.x >= 0f && vector2.x <= 1f && vector2.y >= 0f && vector2.y <= 1f && vector2.z >= 0f)
							{
								SteamScreenshots.TagUser(hScreenshot, steamPlayer.playerID.steamID);
								UnturnedLog.info("Tagged player \"" + steamPlayer.GetLocalDisplayName() + "\" in screenshot");
							}
						}
					}
				}
			}
			Provider.isCapturingScreenshot = false;
			yield break;
		}

		// Token: 0x06003630 RID: 13872 RVA: 0x0010A9B9 File Offset: 0x00108BB9
		public static void RequestScreenshot()
		{
			if (Provider.isCapturingScreenshot)
			{
				return;
			}
			Provider.isCapturingScreenshot = true;
			Provider.steam.StartCoroutine(Provider.steam.CaptureScreenshot());
		}

		// Token: 0x06003631 RID: 13873 RVA: 0x0010A9DE File Offset: 0x00108BDE
		private static void OnSteamScreenshotRequested(ScreenshotRequested_t callback)
		{
			UnturnedLog.info("Steam overlay screenshot requested");
			Provider.RequestScreenshot();
		}

		// Token: 0x1700097A RID: 2426
		// (get) Token: 0x06003632 RID: 13874 RVA: 0x0010A9EF File Offset: 0x00108BEF
		// (set) Token: 0x06003633 RID: 13875 RVA: 0x0010A9F6 File Offset: 0x00108BF6
		public static string language
		{
			get
			{
				return Provider.privateLanguage;
			}
			private set
			{
				Provider.privateLanguage = value;
				Provider.languageIsEnglish = (value == "English");
			}
		}

		// Token: 0x1700097B RID: 2427
		// (get) Token: 0x06003634 RID: 13876 RVA: 0x0010AA0E File Offset: 0x00108C0E
		public static string path
		{
			get
			{
				return Provider._path;
			}
		}

		/// <summary>
		/// Path to directory containing "Editor", "Menu", "Player", "Curse_Words.txt", etc files.
		/// </summary>
		// Token: 0x1700097C RID: 2428
		// (get) Token: 0x06003635 RID: 13877 RVA: 0x0010AA15 File Offset: 0x00108C15
		// (set) Token: 0x06003636 RID: 13878 RVA: 0x0010AA1C File Offset: 0x00108C1C
		public static string localizationRoot { get; private set; }

		// Token: 0x1700097D RID: 2429
		// (get) Token: 0x06003637 RID: 13879 RVA: 0x0010AA24 File Offset: 0x00108C24
		// (set) Token: 0x06003638 RID: 13880 RVA: 0x0010AA2B File Offset: 0x00108C2B
		public static List<string> streamerNames { get; private set; }

		// Token: 0x06003639 RID: 13881 RVA: 0x0010AA33 File Offset: 0x00108C33
		internal static void battlEyeClientPrintMessage(string message)
		{
			UnturnedLog.info("BattlEye client message: {0}", new object[]
			{
				message
			});
		}

		// Token: 0x0600363A RID: 13882 RVA: 0x0010AA49 File Offset: 0x00108C49
		internal static void battlEyeClientRequestRestart(int reason)
		{
			if (reason == 0)
			{
				Provider._connectionFailureInfo = ESteamConnectionFailureInfo.BATTLEYE_BROKEN;
			}
			else if (reason == 1)
			{
				Provider._connectionFailureInfo = ESteamConnectionFailureInfo.BATTLEYE_UPDATE;
			}
			else
			{
				Provider._connectionFailureInfo = ESteamConnectionFailureInfo.BATTLEYE_UNKNOWN;
			}
			Provider.battlEyeHasRequiredRestart = true;
			UnturnedLog.info("BattlEye client requested restart with reason: " + reason.ToString());
		}

		/// <summary>
		/// Called clientside by BattlEye when it needs us to send a packet to the server.
		/// </summary>
		// Token: 0x0600363B RID: 13883 RVA: 0x0010AA88 File Offset: 0x00108C88
		internal static void battlEyeClientSendPacket(IntPtr packetHandle, int length)
		{
			NetMessages.SendMessageToServer(EServerMessage.BattlEye, ENetReliability.Unreliable, delegate(NetPakWriter writer)
			{
				writer.WriteBits((uint)length, Provider.battlEyeBufferSize.bitCount);
				if (!writer.WriteBytes(packetHandle, length))
				{
					UnturnedLog.error("Unable to write BattlEye packet ({0} bytes)", new object[]
					{
						length
					});
				}
			});
		}

		// Token: 0x0600363C RID: 13884 RVA: 0x0010AABC File Offset: 0x00108CBC
		private static void battlEyeServerPrintMessage(string message)
		{
			for (int i = 0; i < Provider.clients.Count; i++)
			{
				SteamPlayer steamPlayer = Provider.clients[i];
				if (steamPlayer != null && !(steamPlayer.player == null) && steamPlayer.player.wantsBattlEyeLogs)
				{
					steamPlayer.player.sendTerminalRelay(message);
				}
			}
			if (CommandWindow.shouldLogAnticheat)
			{
				CommandWindow.Log("BattlEye Server: " + message);
				return;
			}
			UnturnedLog.info("BattlEye Print: {0}", new object[]
			{
				message
			});
		}

		/// <summary>
		/// Event for plugins when BattlEye wants to kick a player.
		/// </summary>
		// Token: 0x140000D2 RID: 210
		// (add) Token: 0x0600363D RID: 13885 RVA: 0x0010AB40 File Offset: 0x00108D40
		// (remove) Token: 0x0600363E RID: 13886 RVA: 0x0010AB74 File Offset: 0x00108D74
		public static event Provider.BattlEyeKickCallback onBattlEyeKick;

		// Token: 0x0600363F RID: 13887 RVA: 0x0010ABA8 File Offset: 0x00108DA8
		private static void broadcastBattlEyeKick(SteamPlayer client, string reason)
		{
			try
			{
				Provider.BattlEyeKickCallback battlEyeKickCallback = Provider.onBattlEyeKick;
				if (battlEyeKickCallback != null)
				{
					battlEyeKickCallback(client, reason);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onBattlEyeKick:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x06003640 RID: 13888 RVA: 0x0010ABEC File Offset: 0x00108DEC
		private static void battlEyeServerKickPlayer(int playerID, string reason)
		{
			foreach (SteamPlayer steamPlayer in Provider.clients)
			{
				if (steamPlayer.battlEyeId == playerID)
				{
					if (steamPlayer.playerID.BypassIntegrityChecks)
					{
						break;
					}
					Provider.broadcastBattlEyeKick(steamPlayer, reason);
					UnturnedLog.info("BattlEye Kick {0} Reason: {1}", new object[]
					{
						steamPlayer.playerID.steamID,
						reason
					});
					if (reason.Length == 18 && reason.StartsWith("Global Ban #"))
					{
						ChatManager.say(steamPlayer.playerID.playerName + " got banned by BattlEye", Color.yellow, false);
					}
					Provider.kick(steamPlayer.playerID.steamID, "BattlEye: " + reason);
					SteamBlacklist.ban(steamPlayer.playerID.steamID, steamPlayer.getIPv4AddressOrZero(), steamPlayer.playerID.GetHwids(), CSteamID.Nil, "(Temporary) BattlEye: " + reason, 60U);
					break;
				}
			}
		}

		/// <summary>
		/// Called serverside by BattlEye when it needs us to send a packet to a player.
		/// </summary>
		// Token: 0x06003641 RID: 13889 RVA: 0x0010AD10 File Offset: 0x00108F10
		private static void battlEyeServerSendPacket(int playerID, IntPtr packetHandle, int length)
		{
			NetMessages.ClientWriteHandler <>9__0;
			for (int i = 0; i < Provider.clients.Count; i++)
			{
				if (Provider.clients[i].battlEyeId == playerID)
				{
					EClientMessage index = EClientMessage.BattlEye;
					ENetReliability reliability = ENetReliability.Unreliable;
					ITransportConnection transportConnection = Provider.clients[i].transportConnection;
					NetMessages.ClientWriteHandler callback;
					if ((callback = <>9__0) == null)
					{
						callback = (<>9__0 = delegate(NetPakWriter writer)
						{
							writer.WriteBits((uint)length, Provider.battlEyeBufferSize.bitCount);
							writer.WriteBytes(packetHandle, length);
						});
					}
					NetMessages.SendMessageToClient(index, reliability, transportConnection, callback);
					return;
				}
			}
		}

		/// <summary>
		/// Call whenever something impacting rich presence changes for example loading a server or changing lobbies.
		/// </summary>
		// Token: 0x06003642 RID: 13890 RVA: 0x0010AD91 File Offset: 0x00108F91
		public static void updateRichPresence()
		{
			if (Dedicator.IsDedicatedServer)
			{
				return;
			}
			Provider.updateSteamRichPresence();
		}

		// Token: 0x06003643 RID: 13891 RVA: 0x0010ADA0 File Offset: 0x00108FA0
		private static void updateSteamRichPresence()
		{
			if (Level.info != null)
			{
				if (Level.isEditor)
				{
					Provider.provider.communityService.setStatus(Provider.localization.format("Rich_Presence_Editing", Level.info.getLocalizedName()));
					SteamFriends.SetRichPresence("steam_display", "#Status_EditingLevel");
					SteamFriends.SetRichPresence("steam_player_group", string.Empty);
				}
				else
				{
					Provider.provider.communityService.setStatus(Provider.localization.format("Rich_Presence_Playing", Level.info.getLocalizedName()));
					if (Provider.isConnected && !Provider.isServer && Provider.server.m_SteamID > 0UL)
					{
						SteamFriends.SetRichPresence("steam_display", "#Status_PlayingMultiplayer");
						SteamFriends.SetRichPresence("steam_player_group", Provider.server.ToString());
					}
					else
					{
						SteamFriends.SetRichPresence("steam_display", "#Status_PlayingSingleplayer");
						SteamFriends.SetRichPresence("steam_player_group", string.Empty);
					}
				}
				SteamFriends.SetRichPresence("level_name", Level.info.getLocalizedName());
				return;
			}
			if (Lobbies.inLobby)
			{
				Provider.provider.communityService.setStatus(Provider.localization.format("Rich_Presence_Lobby"));
				SteamFriends.SetRichPresence("steam_display", "#Status_WaitingInLobby");
				SteamFriends.SetRichPresence("steam_player_group", Lobbies.currentLobby.ToString());
				return;
			}
			Provider.provider.communityService.setStatus(Provider.localization.format("Rich_Presence_Menu"));
			SteamFriends.SetRichPresence("steam_display", "#Status_AtMainMenu");
			SteamFriends.SetRichPresence("steam_player_group", string.Empty);
		}

		// Token: 0x1700097E RID: 2430
		// (get) Token: 0x06003644 RID: 13892 RVA: 0x0010AF47 File Offset: 0x00109147
		public static uint bytesSent
		{
			get
			{
				return Provider._bytesSent;
			}
		}

		// Token: 0x1700097F RID: 2431
		// (get) Token: 0x06003645 RID: 13893 RVA: 0x0010AF4E File Offset: 0x0010914E
		public static uint bytesReceived
		{
			get
			{
				return Provider._bytesReceived;
			}
		}

		// Token: 0x17000980 RID: 2432
		// (get) Token: 0x06003646 RID: 13894 RVA: 0x0010AF55 File Offset: 0x00109155
		public static uint packetsSent
		{
			get
			{
				return Provider._packetsSent;
			}
		}

		// Token: 0x17000981 RID: 2433
		// (get) Token: 0x06003647 RID: 13895 RVA: 0x0010AF5C File Offset: 0x0010915C
		public static uint packetsReceived
		{
			get
			{
				return Provider._packetsReceived;
			}
		}

		/// <summary>
		/// Only used on client.
		/// Information about current game server retrieved through Steam's "A2S" query system.
		/// Available when joining using the Steam server list API (in-game server browser)
		/// or querying the Server's A2S port directly (connect by IP menu), but not when
		/// joining by Steam ID.
		/// </summary>
		// Token: 0x17000982 RID: 2434
		// (get) Token: 0x06003648 RID: 13896 RVA: 0x0010AF63 File Offset: 0x00109163
		public static SteamServerAdvertisement CurrentServerAdvertisement
		{
			get
			{
				return Provider._currentServerAdvertisement;
			}
		}

		// Token: 0x17000983 RID: 2435
		// (get) Token: 0x06003649 RID: 13897 RVA: 0x0010AF6A File Offset: 0x0010916A
		public static ServerConnectParameters CurrentServerConnectParameters
		{
			get
			{
				return Provider._currentServerConnectParameters;
			}
		}

		/// <summary>
		/// On client, is current server protected by VAC?
		/// Set after initial response is received.
		/// </summary>
		// Token: 0x17000984 RID: 2436
		// (get) Token: 0x0600364A RID: 13898 RVA: 0x0010AF71 File Offset: 0x00109171
		internal static bool IsVacActiveOnCurrentServer
		{
			get
			{
				return Provider.isVacActive;
			}
		}

		/// <summary>
		/// On client, is current server protected by BattlEye?
		/// Set after initial response is received.
		/// </summary>
		// Token: 0x17000985 RID: 2437
		// (get) Token: 0x0600364B RID: 13899 RVA: 0x0010AF78 File Offset: 0x00109178
		internal static bool IsBattlEyeActiveOnCurrentServer
		{
			get
			{
				return Provider.isBattlEyeActive;
			}
		}

		// Token: 0x17000986 RID: 2438
		// (get) Token: 0x0600364C RID: 13900 RVA: 0x0010AF7F File Offset: 0x0010917F
		public static CSteamID server
		{
			get
			{
				return Provider._server;
			}
		}

		// Token: 0x17000987 RID: 2439
		// (get) Token: 0x0600364D RID: 13901 RVA: 0x0010AF86 File Offset: 0x00109186
		public static CSteamID client
		{
			get
			{
				return Provider._client;
			}
		}

		// Token: 0x17000988 RID: 2440
		// (get) Token: 0x0600364E RID: 13902 RVA: 0x0010AF8D File Offset: 0x0010918D
		public static CSteamID user
		{
			get
			{
				return Provider._user;
			}
		}

		// Token: 0x17000989 RID: 2441
		// (get) Token: 0x0600364F RID: 13903 RVA: 0x0010AF94 File Offset: 0x00109194
		public static byte[] clientHash
		{
			get
			{
				return Provider._clientHash;
			}
		}

		// Token: 0x1700098A RID: 2442
		// (get) Token: 0x06003650 RID: 13904 RVA: 0x0010AF9B File Offset: 0x0010919B
		public static string clientName
		{
			get
			{
				return Provider._clientName;
			}
		}

		// Token: 0x1700098B RID: 2443
		// (get) Token: 0x06003651 RID: 13905 RVA: 0x0010AFA2 File Offset: 0x001091A2
		public static List<SteamPlayer> clients
		{
			get
			{
				return Provider._clients;
			}
		}

		// Token: 0x06003652 RID: 13906 RVA: 0x0010AFAC File Offset: 0x001091AC
		public static PooledTransportConnectionList GatherClientConnections()
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				pooledTransportConnectionList.Add(steamPlayer.transportConnection);
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x06003653 RID: 13907 RVA: 0x0010B00C File Offset: 0x0010920C
		[Obsolete("Replaced by GatherClientConnections")]
		public static IEnumerable<ITransportConnection> EnumerateClients()
		{
			return Provider.GatherClientConnections();
		}

		// Token: 0x06003654 RID: 13908 RVA: 0x0010B014 File Offset: 0x00109214
		public static PooledTransportConnectionList GatherClientConnectionsMatchingPredicate(Predicate<SteamPlayer> predicate)
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				if (predicate(steamPlayer))
				{
					pooledTransportConnectionList.Add(steamPlayer.transportConnection);
				}
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x06003655 RID: 13909 RVA: 0x0010B07C File Offset: 0x0010927C
		[Obsolete("Replaced by GatherClientConnectionsMatchingPredicate")]
		public static IEnumerable<ITransportConnection> EnumerateClients_Predicate(Predicate<SteamPlayer> predicate)
		{
			return Provider.GatherClientConnectionsMatchingPredicate(predicate);
		}

		// Token: 0x06003656 RID: 13910 RVA: 0x0010B084 File Offset: 0x00109284
		public static PooledTransportConnectionList GatherClientConnectionsWithinSphere(Vector3 position, float radius)
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			float num = radius * radius;
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				if (steamPlayer.player != null && (steamPlayer.player.transform.position - position).sqrMagnitude < num)
				{
					pooledTransportConnectionList.Add(steamPlayer.transportConnection);
				}
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x06003657 RID: 13911 RVA: 0x0010B118 File Offset: 0x00109318
		[Obsolete("Replaced by GatherClientConnectionsWithinSphere")]
		public static IEnumerable<ITransportConnection> EnumerateClients_WithinSphere(Vector3 position, float radius)
		{
			return Provider.GatherClientConnectionsWithinSphere(position, radius);
		}

		// Token: 0x06003658 RID: 13912 RVA: 0x0010B124 File Offset: 0x00109324
		public static PooledTransportConnectionList GatherRemoteClientConnectionsWithinSphere(Vector3 position, float radius)
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			float num = radius * radius;
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				if (!steamPlayer.IsLocalServerHost && steamPlayer.player != null && (steamPlayer.player.transform.position - position).sqrMagnitude < num)
				{
					pooledTransportConnectionList.Add(steamPlayer.transportConnection);
				}
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x06003659 RID: 13913 RVA: 0x0010B1C0 File Offset: 0x001093C0
		[Obsolete("Replaced by GatherRemoteClientConnectionsWithinSphere")]
		public static IEnumerable<ITransportConnection> EnumerateClients_RemoteWithinSphere(Vector3 position, float radius)
		{
			return Provider.GatherRemoteClientConnectionsWithinSphere(position, radius);
		}

		// Token: 0x0600365A RID: 13914 RVA: 0x0010B1CC File Offset: 0x001093CC
		public static PooledTransportConnectionList GatherRemoteClientConnections()
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				if (!steamPlayer.IsLocalServerHost)
				{
					pooledTransportConnectionList.Add(steamPlayer.transportConnection);
				}
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x0600365B RID: 13915 RVA: 0x0010B234 File Offset: 0x00109434
		[Obsolete("Replaced by GatherRemoteClientConnections")]
		public static IEnumerable<ITransportConnection> EnumerateClients_Remote()
		{
			return Provider.GatherRemoteClientConnections();
		}

		// Token: 0x0600365C RID: 13916 RVA: 0x0010B23C File Offset: 0x0010943C
		public static PooledTransportConnectionList GatherRemoteClientConnectionsMatchingPredicate(Predicate<SteamPlayer> predicate)
		{
			PooledTransportConnectionList pooledTransportConnectionList = TransportConnectionListPool.Get();
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				if (!steamPlayer.IsLocalServerHost && predicate(steamPlayer))
				{
					pooledTransportConnectionList.Add(steamPlayer.transportConnection);
				}
			}
			return pooledTransportConnectionList;
		}

		// Token: 0x0600365D RID: 13917 RVA: 0x0010B2AC File Offset: 0x001094AC
		[Obsolete("Replaced by GatherRemoteClientsMatchingPredicate")]
		public static IEnumerable<ITransportConnection> EnumerateClients_RemotePredicate(Predicate<SteamPlayer> predicate)
		{
			return Provider.GatherRemoteClientConnectionsMatchingPredicate(predicate);
		}

		/// <summary>
		/// Exposed for Rocket transition to modules backwards compatibility.
		/// </summary>
		// Token: 0x1700098C RID: 2444
		// (get) Token: 0x0600365E RID: 13918 RVA: 0x0010B2B4 File Offset: 0x001094B4
		[Obsolete]
		public static List<SteamPlayer> players
		{
			get
			{
				return Provider.clients;
			}
		}

		// Token: 0x1700098D RID: 2445
		// (get) Token: 0x0600365F RID: 13919 RVA: 0x0010B2BB File Offset: 0x001094BB
		public static bool isServer
		{
			get
			{
				return Provider._isServer;
			}
		}

		// Token: 0x1700098E RID: 2446
		// (get) Token: 0x06003660 RID: 13920 RVA: 0x0010B2C2 File Offset: 0x001094C2
		public static bool isClient
		{
			get
			{
				return Provider._isClient;
			}
		}

		// Token: 0x1700098F RID: 2447
		// (get) Token: 0x06003661 RID: 13921 RVA: 0x0010B2C9 File Offset: 0x001094C9
		public static bool isPro
		{
			get
			{
				return Provider._isPro;
			}
		}

		// Token: 0x17000990 RID: 2448
		// (get) Token: 0x06003662 RID: 13922 RVA: 0x0010B2D0 File Offset: 0x001094D0
		public static bool isConnected
		{
			get
			{
				return Provider._isConnected;
			}
		}

		// Token: 0x06003663 RID: 13923 RVA: 0x0010B2D8 File Offset: 0x001094D8
		private static bool doServerItemsMatchAdvertisement(List<PublishedFileId_t> pendingWorkshopItems)
		{
			if (Provider.waitingForExpectedWorkshopItems == null)
			{
				return true;
			}
			if (Provider.waitingForExpectedWorkshopItems.Count < pendingWorkshopItems.Count)
			{
				return false;
			}
			foreach (PublishedFileId_t item in pendingWorkshopItems)
			{
				if (!Provider.waitingForExpectedWorkshopItems.Contains(item))
				{
					return false;
				}
			}
			return true;
		}

		// Token: 0x06003664 RID: 13924 RVA: 0x0010B350 File Offset: 0x00109550
		internal static void receiveWorkshopResponse(Provider.CachedWorkshopResponse response)
		{
			Provider.authorityHoliday = response.holiday;
			Provider.currentServerWorkshopResponse = response;
			Provider.isWaitingForWorkshopResponse = false;
			Provider.serverName = response.serverName;
			Provider.map = response.levelName;
			Provider.isPvP = response.isPvP;
			Provider.mode = response.gameMode;
			Provider.cameraMode = response.cameraMode;
			Provider.maxPlayers = response.maxPlayers;
			Provider.isVacActive = response.isVACSecure;
			Provider.isBattlEyeActive = response.isBattlEyeSecure;
			List<PublishedFileId_t> list = new List<PublishedFileId_t>(response.requiredFiles.Count);
			foreach (Provider.ServerRequiredWorkshopFile serverRequiredWorkshopFile in response.requiredFiles)
			{
				if (serverRequiredWorkshopFile.fileId != 0UL)
				{
					list.Add(new PublishedFileId_t(serverRequiredWorkshopFile.fileId));
				}
			}
			Provider.provider.workshopService.resetServerInvalidItems();
			if (Provider.CurrentServerAdvertisement != null)
			{
				if (!string.IsNullOrEmpty(Provider.CurrentServerAdvertisement.map) && !string.Equals(Provider.CurrentServerAdvertisement.map, response.levelName, StringComparison.OrdinalIgnoreCase))
				{
					Provider._connectionFailureInfo = ESteamConnectionFailureInfo.SERVER_MAP_ADVERTISEMENT_MISMATCH;
					Provider.RequestDisconnect(string.Concat(new string[]
					{
						"server map advertisement mismatch (Advertisement: \"",
						Provider.CurrentServerAdvertisement.map,
						"\" Response: \"",
						response.levelName,
						"\")"
					}));
					return;
				}
				if (Provider.CurrentServerAdvertisement.IsBattlEyeSecure != response.isBattlEyeSecure)
				{
					Provider._connectionFailureInfo = ESteamConnectionFailureInfo.SERVER_BATTLEYE_ADVERTISEMENT_MISMATCH;
					Provider.RequestDisconnect(string.Format("server BE advertisement mismatch (Advertisement: {0} Response: {1})", Provider.CurrentServerAdvertisement.IsBattlEyeSecure, response.isBattlEyeSecure));
					return;
				}
				if (Provider.CurrentServerAdvertisement.maxPlayers != (int)response.maxPlayers)
				{
					Provider._connectionFailureInfo = ESteamConnectionFailureInfo.SERVER_MAXPLAYERS_ADVERTISEMENT_MISMATCH;
					Provider.RequestDisconnect(string.Format("server max players advertisement mismatch (Advertisement: {0} Response: {1})", Provider.CurrentServerAdvertisement.maxPlayers, response.maxPlayers));
					return;
				}
				if (Provider.CurrentServerAdvertisement.cameraMode != response.cameraMode)
				{
					Provider._connectionFailureInfo = ESteamConnectionFailureInfo.SERVER_CAMERAMODE_ADVERTISEMENT_MISMATCH;
					Provider.RequestDisconnect(string.Format("server camera mode advertisement mismatch (Advertisement: {0} Response: {1})", Provider.CurrentServerAdvertisement.cameraMode, response.cameraMode));
					return;
				}
				if (Provider.CurrentServerAdvertisement.isPvP != response.isPvP)
				{
					Provider._connectionFailureInfo = ESteamConnectionFailureInfo.SERVER_PVP_ADVERTISEMENT_MISMATCH;
					Provider.RequestDisconnect(string.Format("server PvP advertisement mismatch (Advertisement: {0} Response: {1})", Provider.CurrentServerAdvertisement.isPvP, response.isPvP));
					return;
				}
			}
			if (list.Count < 1)
			{
				UnturnedLog.info("Server specified no workshop items, launching");
				Provider.launch();
				return;
			}
			SteamServerAdvertisement currentServerAdvertisement = Provider.CurrentServerAdvertisement;
			if ((currentServerAdvertisement == null || currentServerAdvertisement.isWorkshop) && Provider.doServerItemsMatchAdvertisement(list))
			{
				Provider.canCurrentlyHandleClientTransportFailure = false;
				UnturnedLog.info("Server specified {0} workshop item(s), querying details", new object[]
				{
					list.Count
				});
				Provider.provider.workshopService.queryServerWorkshopItems(list, response.ip);
				return;
			}
			Provider._connectionFailureInfo = ESteamConnectionFailureInfo.WORKSHOP_ADVERTISEMENT_MISMATCH;
			Provider.RequestDisconnect("workshop advertisement mismatch");
		}

		/// <summary>
		/// Only safe to use serverside.
		/// Get the list of workshop ids that a client needs to download when joining.
		/// </summary>
		// Token: 0x06003665 RID: 13925 RVA: 0x0010B64C File Offset: 0x0010984C
		public static List<ulong> getServerWorkshopFileIDs()
		{
			return Provider._serverWorkshopFileIDs;
		}

		/// <summary>
		/// Only safe to use serverside.
		/// Lets clients know that this workshop id is being used on the server, and that they need to download it when joining.
		/// </summary>
		// Token: 0x06003666 RID: 13926 RVA: 0x0010B653 File Offset: 0x00109853
		public static void registerServerUsingWorkshopFileId(ulong id)
		{
			Provider.registerServerUsingWorkshopFileId(id, 0U);
		}

		// Token: 0x06003667 RID: 13927 RVA: 0x0010B65C File Offset: 0x0010985C
		internal static void registerServerUsingWorkshopFileId(ulong id, uint timestamp)
		{
			if (Provider._serverWorkshopFileIDs.Contains(id))
			{
				return;
			}
			Provider._serverWorkshopFileIDs.Add(id);
			Provider.ServerRequiredWorkshopFile item = new Provider.ServerRequiredWorkshopFile
			{
				fileId = id,
				timestamp = DateTimeEx.FromUtcUnixTimeSeconds(timestamp)
			};
			UnturnedLog.info(string.Format("Workshop file {0} requiring timestamp {1}", id, item.timestamp.ToLocalTime()));
			Provider.serverRequiredWorkshopFiles.Add(item);
		}

		// Token: 0x17000991 RID: 2449
		// (get) Token: 0x06003668 RID: 13928 RVA: 0x0010B6D2 File Offset: 0x001098D2
		public static bool isLoading
		{
			get
			{
				return Provider.isLoadingUGC;
			}
		}

		// Token: 0x17000992 RID: 2450
		// (get) Token: 0x06003669 RID: 13929 RVA: 0x0010B6D9 File Offset: 0x001098D9
		[Obsolete]
		public static int channels
		{
			get
			{
				return 0;
			}
		}

		/// <summary>
		/// Channel id was 32-bits, but now that it is in the RPC header it can be 8-bits since there never that many
		/// players online. The "manager" components are on channel 1, and each player has a channel.
		/// </summary>
		// Token: 0x0600366A RID: 13930 RVA: 0x0010B6DC File Offset: 0x001098DC
		private static int allocPlayerChannelId()
		{
			for (int i = 0; i < 255; i++)
			{
				int num = Provider.nextPlayerChannelId;
				Provider.nextPlayerChannelId++;
				if (Provider.nextPlayerChannelId > 255)
				{
					Provider.nextPlayerChannelId = 2;
				}
				if (Provider.findChannelComponent(num) == null)
				{
					return num;
				}
			}
			CommandWindow.LogErrorFormat("Fatal error! Ran out of player RPC channel IDs", Array.Empty<object>());
			Provider.shutdown(1, "Fatal error! Ran out of player RPC channel IDs");
			return 2;
		}

		// Token: 0x17000993 RID: 2451
		// (get) Token: 0x0600366B RID: 13931 RVA: 0x0010B748 File Offset: 0x00109948
		// (set) Token: 0x0600366C RID: 13932 RVA: 0x0010B74F File Offset: 0x0010994F
		public static ESteamConnectionFailureInfo connectionFailureInfo
		{
			get
			{
				return Provider._connectionFailureInfo;
			}
			set
			{
				Provider._connectionFailureInfo = value;
			}
		}

		// Token: 0x17000994 RID: 2452
		// (get) Token: 0x0600366D RID: 13933 RVA: 0x0010B757 File Offset: 0x00109957
		// (set) Token: 0x0600366E RID: 13934 RVA: 0x0010B75E File Offset: 0x0010995E
		public static string connectionFailureReason
		{
			get
			{
				return Provider._connectionFailureReason;
			}
			set
			{
				Provider._connectionFailureReason = value;
			}
		}

		// Token: 0x17000995 RID: 2453
		// (get) Token: 0x0600366F RID: 13935 RVA: 0x0010B766 File Offset: 0x00109966
		public static uint connectionFailureDuration
		{
			get
			{
				return Provider._connectionFailureDuration;
			}
		}

		// Token: 0x17000996 RID: 2454
		// (get) Token: 0x06003670 RID: 13936 RVA: 0x0010B76D File Offset: 0x0010996D
		public static List<SteamChannel> receivers
		{
			get
			{
				return Provider._receivers;
			}
		}

		// Token: 0x06003671 RID: 13937 RVA: 0x0010B774 File Offset: 0x00109974
		private static int allocBattlEyePlayerId()
		{
			int result = Provider.nextBattlEyePlayerId;
			Provider.nextBattlEyePlayerId++;
			return result;
		}

		// Token: 0x06003672 RID: 13938 RVA: 0x0010B787 File Offset: 0x00109987
		public static void resetConnectionFailure()
		{
			Provider._connectionFailureInfo = ESteamConnectionFailureInfo.NONE;
			Provider._connectionFailureReason = "";
			Provider._connectionFailureDuration = 0U;
		}

		// Token: 0x06003673 RID: 13939 RVA: 0x0010B79F File Offset: 0x0010999F
		[Conditional("LOG_NETCHANNEL")]
		private static void LogNetChannel(string format, params object[] args)
		{
			UnturnedLog.info(format, args);
		}

		// Token: 0x06003674 RID: 13940 RVA: 0x0010B7A8 File Offset: 0x001099A8
		public static void openChannel(SteamChannel receiver)
		{
			Provider.receivers.Add(receiver);
		}

		// Token: 0x06003675 RID: 13941 RVA: 0x0010B7B5 File Offset: 0x001099B5
		public static void closeChannel(SteamChannel receiver)
		{
			Provider.receivers.RemoveFast(receiver);
		}

		// Token: 0x06003676 RID: 13942 RVA: 0x0010B7C4 File Offset: 0x001099C4
		internal static SteamChannel findChannelComponent(int id)
		{
			for (int i = Provider.receivers.Count - 1; i >= 0; i--)
			{
				SteamChannel steamChannel = Provider.receivers[i];
				if (steamChannel == null)
				{
					Provider.receivers.RemoveAtFast(i);
				}
				else if (steamChannel.id == id)
				{
					return steamChannel;
				}
			}
			return null;
		}

		/// <summary>
		/// Should the network transport layer accept incoming connections?
		/// If both the queue and connected slots are full then incoming connections are ignored.
		/// </summary>
		// Token: 0x17000997 RID: 2455
		// (get) Token: 0x06003677 RID: 13943 RVA: 0x0010B816 File Offset: 0x00109A16
		public static bool hasRoomForNewConnection
		{
			get
			{
				return Provider.clients.Count < (int)Provider.maxPlayers || Provider.pending.Count < (int)Provider.queueSize;
			}
		}

		/// <summary>
		/// Find player in the queue associated with a client connection.
		/// </summary>
		// Token: 0x06003678 RID: 13944 RVA: 0x0010B83C File Offset: 0x00109A3C
		public static SteamPending findPendingPlayer(ITransportConnection transportConnection)
		{
			if (transportConnection == null)
			{
				return null;
			}
			foreach (SteamPending steamPending in Provider.pending)
			{
				if (transportConnection.Equals(steamPending.transportConnection))
				{
					return steamPending;
				}
			}
			return null;
		}

		// Token: 0x06003679 RID: 13945 RVA: 0x0010B8A4 File Offset: 0x00109AA4
		internal static SteamPending findPendingPlayerBySteamId(CSteamID steamId)
		{
			foreach (SteamPending steamPending in Provider.pending)
			{
				if (steamPending.playerID.steamID == steamId)
				{
					return steamPending;
				}
			}
			return null;
		}

		/// <summary>
		/// Find player associated with a client connection.
		/// </summary>
		// Token: 0x0600367A RID: 13946 RVA: 0x0010B90C File Offset: 0x00109B0C
		public static SteamPlayer findPlayer(ITransportConnection transportConnection)
		{
			if (transportConnection == null)
			{
				return null;
			}
			foreach (SteamPlayer steamPlayer in Provider.clients)
			{
				if (transportConnection.Equals(steamPlayer.transportConnection))
				{
					return steamPlayer;
				}
			}
			return null;
		}

		/// <summary>
		/// Find net transport layer connection associated with a client steam id. This could be a pending player in the
		/// queue, or a fully connected player.
		/// </summary>
		// Token: 0x0600367B RID: 13947 RVA: 0x0010B974 File Offset: 0x00109B74
		public static ITransportConnection findTransportConnection(CSteamID steamId)
		{
			foreach (SteamPlayer steamPlayer in Provider.clients)
			{
				if (steamPlayer.playerID.steamID == steamId)
				{
					return steamPlayer.transportConnection;
				}
			}
			foreach (SteamPending steamPending in Provider.pending)
			{
				if (steamPending.playerID.steamID == steamId)
				{
					return steamPending.transportConnection;
				}
			}
			return null;
		}

		/// <summary>
		/// Find player steam id associated with connection, otherwise nil if not found.
		/// </summary>
		// Token: 0x0600367C RID: 13948 RVA: 0x0010BA38 File Offset: 0x00109C38
		public static CSteamID findTransportConnectionSteamId(ITransportConnection transportConnection)
		{
			SteamPlayer steamPlayer = Provider.findPlayer(transportConnection);
			if (steamPlayer != null)
			{
				return steamPlayer.playerID.steamID;
			}
			SteamPending steamPending = Provider.findPendingPlayer(transportConnection);
			if (steamPending != null)
			{
				return steamPending.playerID.steamID;
			}
			return CSteamID.Nil;
		}

		// Token: 0x0600367D RID: 13949 RVA: 0x0010BA76 File Offset: 0x00109C76
		internal static NetId ClaimNetIdBlockForNewPlayer()
		{
			return NetIdRegistry.ClaimBlock(16U);
		}

		// Token: 0x0600367E RID: 13950 RVA: 0x0010BA80 File Offset: 0x00109C80
		internal static SteamPlayer addPlayer(ITransportConnection transportConnection, NetId netId, SteamPlayerID playerID, Vector3 point, byte angle, bool isPro, bool isAdmin, int channel, byte face, byte hair, byte beard, Color skin, Color color, Color markerColor, bool hand, int shirtItem, int pantsItem, int hatItem, int backpackItem, int vestItem, int maskItem, int glassesItem, int[] skinItems, string[] skinTags, string[] skinDynamicProps, EPlayerSkillset skillset, string language, CSteamID lobbyID, EClientPlatform clientPlatform)
		{
			if (!Dedicator.IsDedicatedServer && playerID.steamID != Provider.client)
			{
				SteamFriends.SetPlayedWith(playerID.steamID);
			}
			if (playerID.steamID == Provider.client && Level.placeholderAudioListener != null)
			{
				UnityEngine.Object.Destroy(Level.placeholderAudioListener);
				Level.placeholderAudioListener = null;
			}
			Transform transform = null;
			try
			{
				transform = Provider.gameMode.getPlayerGameObject(playerID).transform;
				transform.position = point;
				transform.rotation = Quaternion.Euler(0f, (float)(angle * 2), 0f);
			}
			catch (Exception e)
			{
				UnturnedLog.error("Exception thrown when getting player from game mode:");
				UnturnedLog.exception(e);
			}
			SteamPlayer steamPlayer = null;
			try
			{
				steamPlayer = new SteamPlayer(transportConnection, netId, playerID, transform, isPro, isAdmin, channel, face, hair, beard, skin, color, markerColor, hand, shirtItem, pantsItem, hatItem, backpackItem, vestItem, maskItem, glassesItem, skinItems, skinTags, skinDynamicProps, skillset, language, lobbyID, clientPlatform);
				Provider.clients.Add(steamPlayer);
			}
			catch (Exception e2)
			{
				UnturnedLog.error("Exception thrown when adding player:");
				UnturnedLog.exception(e2);
			}
			Provider.updateRichPresence();
			Provider.broadcastEnemyConnected(steamPlayer);
			return steamPlayer;
		}

		// Token: 0x0600367F RID: 13951 RVA: 0x0010BBAC File Offset: 0x00109DAC
		internal static void removePlayer(byte index)
		{
			if (index < 0 || (int)index >= Provider.clients.Count)
			{
				UnturnedLog.error("Failed to find player: " + index.ToString());
				return;
			}
			SteamPlayer steamPlayer = Provider.clients[(int)index];
			if (Provider.battlEyeServerHandle != IntPtr.Zero && Provider.battlEyeServerRunData != null && Provider.battlEyeServerRunData.pfnChangePlayerStatus != null)
			{
				Provider.battlEyeServerRunData.pfnChangePlayerStatus(steamPlayer.battlEyeId, -1);
			}
			if (Dedicator.IsDedicatedServer)
			{
				steamPlayer.transportConnection.CloseConnection();
			}
			Provider.broadcastEnemyDisconnected(steamPlayer);
			steamPlayer.player.ReleaseNetIdBlock();
			if (steamPlayer.model != null)
			{
				UnityEngine.Object.Destroy(steamPlayer.model.gameObject);
			}
			NetIdRegistry.Release(steamPlayer.GetNetId());
			Provider.clients.RemoveAt((int)index);
			Provider.verifyNextPlayerInQueue();
			Provider.updateRichPresence();
		}

		// Token: 0x06003680 RID: 13952 RVA: 0x0010BC8C File Offset: 0x00109E8C
		private static void replicateRemovePlayer(CSteamID skipSteamID, byte removalIndex)
		{
			NetMessages.SendMessageToClients(EClientMessage.PlayerDisconnected, ENetReliability.Reliable, Provider.GatherRemoteClientConnectionsMatchingPredicate((SteamPlayer potentialRecipient) => potentialRecipient.playerID.steamID != skipSteamID), delegate(NetPakWriter writer)
			{
				writer.WriteUInt8(removalIndex);
			});
		}

		/// <summary>
		/// If there's space on the server, asks player at front of queue for their verification to begin playing.
		/// </summary>
		// Token: 0x06003681 RID: 13953 RVA: 0x0010BCD4 File Offset: 0x00109ED4
		internal static void verifyNextPlayerInQueue()
		{
			if (Provider.pending.Count < 1)
			{
				return;
			}
			if (Provider.clients.Count >= (int)Provider.maxPlayers)
			{
				return;
			}
			SteamPending steamPending = Provider.pending[0];
			if (steamPending.hasSentVerifyPacket)
			{
				return;
			}
			steamPending.sendVerifyPacket();
		}

		// Token: 0x06003682 RID: 13954 RVA: 0x0010BD1C File Offset: 0x00109F1C
		[Obsolete]
		private static bool isUnreliable(ESteamPacket type)
		{
			return type == ESteamPacket.UPDATE_UNRELIABLE_BUFFER || type - ESteamPacket.UPDATE_UNRELIABLE_CHUNK_BUFFER <= 1;
		}

		// Token: 0x06003683 RID: 13955 RVA: 0x0010BD2B File Offset: 0x00109F2B
		[Obsolete]
		public static bool isChunk(ESteamPacket packet)
		{
			return packet - ESteamPacket.UPDATE_RELIABLE_CHUNK_BUFFER <= 1;
		}

		// Token: 0x06003684 RID: 13956 RVA: 0x0010BD36 File Offset: 0x00109F36
		[Obsolete]
		private static bool isUpdate(ESteamPacket packet)
		{
			return packet <= ESteamPacket.UPDATE_VOICE;
		}

		// Token: 0x06003685 RID: 13957 RVA: 0x0010BD40 File Offset: 0x00109F40
		internal static void resetChannels()
		{
			Provider._bytesSent = 0U;
			Provider._bytesReceived = 0U;
			Provider._packetsSent = 0U;
			Provider._packetsReceived = 0U;
			Provider._clients.Clear();
			Provider.pending.Clear();
			NetIdRegistry.Clear();
			NetInvocationDeferralRegistry.Clear();
			ClientAssetIntegrity.Clear();
			ItemManager.ClearNetworkStuff();
			BarricadeManager.ClearNetworkStuff();
			StructureManager.ClearNetworkStuff();
		}

		// Token: 0x06003686 RID: 13958 RVA: 0x0010BD98 File Offset: 0x00109F98
		private static void loadPlayerSpawn(SteamPlayerID playerID, out Vector3 point, out byte angle, out EPlayerStance initialStance)
		{
			point = Vector3.zero;
			angle = 0;
			initialStance = EPlayerStance.STAND;
			bool flag = false;
			if (PlayerSavedata.fileExists(playerID, "/Player/Player.dat") && Level.info.type == ELevelType.SURVIVAL)
			{
				Block block = PlayerSavedata.readBlock(playerID, "/Player/Player.dat", 1);
				point = block.readSingleVector3() + new Vector3(0f, 0.01f, 0f);
				angle = block.readByte();
				if (!point.IsFinite())
				{
					flag = true;
					UnturnedLog.info("Reset {0} spawn position ({1}) because it was NaN or infinity", new object[]
					{
						playerID,
						point
					});
				}
				else if (point.y > Level.HEIGHT)
				{
					UnturnedLog.info("Clamped {0} spawn position ({1}) because it was above the world height limit ({2})", new object[]
					{
						playerID,
						point,
						Level.HEIGHT
					});
					point.y = Level.HEIGHT - 10f;
				}
				else if (!PlayerStance.getStanceForPosition(point, ref initialStance))
				{
					UnturnedLog.info("Reset {0} spawn position ({1}) because it was obstructed", new object[]
					{
						playerID,
						point
					});
					flag = true;
				}
			}
			else
			{
				flag = true;
			}
			try
			{
				if (Provider.onLoginSpawning != null)
				{
					float num = (float)(angle * 2);
					Provider.onLoginSpawning(playerID, ref point, ref num, ref initialStance, ref flag);
					angle = (byte)(num / 2f);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onLoginSpawning:");
				UnturnedLog.exception(e);
			}
			if (flag)
			{
				PlayerSpawnpoint spawn = LevelPlayers.getSpawn(false);
				point = spawn.point + new Vector3(0f, 0.5f, 0f);
				angle = (byte)(spawn.angle / 2f);
			}
		}

		// Token: 0x06003687 RID: 13959 RVA: 0x0010BF58 File Offset: 0x0010A158
		private static void ResetClientTransportFailure()
		{
			Provider.canCurrentlyHandleClientTransportFailure = true;
			Provider.hasPendingClientTransportFailure = false;
			Provider.pendingClientTransportFailureMessage = null;
		}

		// Token: 0x06003688 RID: 13960 RVA: 0x0010BF6C File Offset: 0x0010A16C
		private static void TriggerDisconnectFromClientTransportFailure()
		{
			Provider.hasPendingClientTransportFailure = false;
			Provider._connectionFailureInfo = ESteamConnectionFailureInfo.CUSTOM;
			Provider._connectionFailureReason = Provider.pendingClientTransportFailureMessage;
			Provider.RequestDisconnect("Client transport failure: \"" + Provider.pendingClientTransportFailureMessage + "\"");
		}

		// Token: 0x06003689 RID: 13961 RVA: 0x0010BFA0 File Offset: 0x0010A1A0
		private static void onLevelLoaded(int level)
		{
			if (level == 2)
			{
				Provider.isLoadingUGC = false;
				if (Provider.isConnected)
				{
					if (Provider.isServer)
					{
						if (Provider.isClient)
						{
							SteamPlayerID steamPlayerID = new SteamPlayerID(Provider.client, Characters.selected, Provider.clientName, Characters.active.name, Characters.active.nick, Characters.active.group);
							Vector3 point;
							byte angle;
							EPlayerStance initialStance;
							Provider.loadPlayerSpawn(steamPlayerID, out point, out angle, out initialStance);
							int inventoryItem = Provider.provider.economyService.getInventoryItem(Characters.active.packageShirt);
							int inventoryItem2 = Provider.provider.economyService.getInventoryItem(Characters.active.packagePants);
							int inventoryItem3 = Provider.provider.economyService.getInventoryItem(Characters.active.packageHat);
							int inventoryItem4 = Provider.provider.economyService.getInventoryItem(Characters.active.packageBackpack);
							int inventoryItem5 = Provider.provider.economyService.getInventoryItem(Characters.active.packageVest);
							int inventoryItem6 = Provider.provider.economyService.getInventoryItem(Characters.active.packageMask);
							int inventoryItem7 = Provider.provider.economyService.getInventoryItem(Characters.active.packageGlasses);
							int[] array = new int[Characters.packageSkins.Count];
							for (int i = 0; i < array.Length; i++)
							{
								array[i] = Provider.provider.economyService.getInventoryItem(Characters.packageSkins[i]);
							}
							string[] array2 = new string[Characters.packageSkins.Count];
							for (int j = 0; j < array2.Length; j++)
							{
								array2[j] = Provider.provider.economyService.getInventoryTags(Characters.packageSkins[j]);
							}
							string[] array3 = new string[Characters.packageSkins.Count];
							for (int k = 0; k < array3.Length; k++)
							{
								array3[k] = Provider.provider.economyService.getInventoryDynamicProps(Characters.packageSkins[k]);
							}
							TransportConnection_Loopback transportConnection_Loopback = TransportConnection_Loopback.Create();
							NetId netId = Provider.ClaimNetIdBlockForNewPlayer();
							SteamPlayer steamPlayer = Provider.addPlayer(transportConnection_Loopback, netId, steamPlayerID, point, angle, Provider.isPro, true, Provider.allocPlayerChannelId(), Characters.active.face, Characters.active.hair, Characters.active.beard, Characters.active.skin, Characters.active.color, Characters.active.markerColor, Characters.active.hand, inventoryItem, inventoryItem2, inventoryItem3, inventoryItem4, inventoryItem5, inventoryItem6, inventoryItem7, array, array2, array3, Characters.active.skillset, Provider.language, Lobbies.currentLobby, EClientPlatform.Windows);
							steamPlayer.player.stance.initialStance = initialStance;
							steamPlayer.player.InitializePlayer();
							steamPlayer.player.SendInitialPlayerState(steamPlayer);
							Lobbies.leaveLobby();
							Provider.updateRichPresence();
							try
							{
								Provider.ServerConnected serverConnected = Provider.onServerConnected;
								if (serverConnected != null)
								{
									serverConnected(steamPlayerID.steamID);
								}
								return;
							}
							catch (Exception e)
							{
								UnturnedLog.warn("Plugin raised an exception from onServerConnected:");
								UnturnedLog.exception(e);
								return;
							}
						}
						if (Dedicator.IsDedicatedServer)
						{
							CommandWindow.Log("//////////////////////////////////////////////////////");
							CommandWindow.Log(Provider.localization.format("ServerCode", SteamGameServer.GetSteamID()));
							CommandWindow.Log(Provider.localization.format("ServerCodeDetails"));
							CommandWindow.Log(Provider.localization.format("ServerCodeCopy", "CopyServerCode"));
							CommandWindow.Log("//////////////////////////////////////////////////////");
							return;
						}
					}
					else
					{
						if (Provider.hasPendingClientTransportFailure)
						{
							UnturnedLog.info("Now able to handle client transport failure that occurred during level load");
							Provider.TriggerDisconnectFromClientTransportFailure();
							return;
						}
						Provider.canCurrentlyHandleClientTransportFailure = true;
						EClientPlatform clientPlatform = EClientPlatform.Windows;
						Provider.critMods.Clear();
						Provider.modBuilder.Length = 0;
						ModuleHook.getRequiredModules(Provider.critMods);
						for (int l = 0; l < Provider.critMods.Count; l++)
						{
							Provider.modBuilder.Append(Provider.critMods[l].config.Name);
							Provider.modBuilder.Append(",");
							Provider.modBuilder.Append(Provider.critMods[l].config.Version_Internal);
							if (l < Provider.critMods.Count - 1)
							{
								Provider.modBuilder.Append(";");
							}
						}
						UnturnedLog.info("Ready to connect");
						Provider.isWaitingForConnectResponse = true;
						Provider.sentConnectRequestTime = Time.realtimeSinceStartup;
						NetMessages.SendMessageToServer(EServerMessage.ReadyToConnect, ENetReliability.Reliable, delegate(NetPakWriter writer)
						{
							writer.WriteUInt8(Characters.selected);
							writer.WriteString(Provider.clientName, 11);
							writer.WriteString(Characters.active.name, 11);
							writer.WriteBytes(Provider._serverPasswordHash, 20);
							writer.WriteBytes(Level.hash, 20);
							writer.WriteBytes(ReadWrite.readData(), 20);
							writer.WriteBytes(ResourceHash.localHash, 20);
							writer.WriteEnum(clientPlatform);
							writer.WriteUInt32(Provider.APP_VERSION_PACKED);
							writer.WriteBit(Provider.isPro);
							SteamServerAdvertisement currentServerAdvertisement = Provider.CurrentServerAdvertisement;
							writer.WriteUInt16(MathfEx.ClampToUShort((currentServerAdvertisement != null) ? currentServerAdvertisement.ping : 1));
							writer.WriteString(Characters.active.nick, 11);
							writer.WriteSteamID(Characters.active.group);
							writer.WriteUInt8(Characters.active.face);
							writer.WriteUInt8(Characters.active.hair);
							writer.WriteUInt8(Characters.active.beard);
							writer.WriteColor32RGB(Characters.active.skin);
							writer.WriteColor32RGB(Characters.active.color);
							writer.WriteColor32RGB(Characters.active.markerColor);
							writer.WriteBit(Characters.active.hand);
							writer.WriteUInt64(Characters.active.packageShirt);
							writer.WriteUInt64(Characters.active.packagePants);
							writer.WriteUInt64(Characters.active.packageHat);
							writer.WriteUInt64(Characters.active.packageBackpack);
							writer.WriteUInt64(Characters.active.packageVest);
							writer.WriteUInt64(Characters.active.packageMask);
							writer.WriteUInt64(Characters.active.packageGlasses);
							writer.WriteList(Characters.packageSkins, new SystemNetPakWriterEx.WriteListItem<ulong>(writer.WriteUInt64), Provider.MAX_SKINS_LENGTH);
							writer.WriteEnum(Characters.active.skillset);
							writer.WriteString(Provider.modBuilder.ToString(), 11);
							writer.WriteString(Provider.language, 11);
							writer.WriteSteamID(Lobbies.currentLobby);
							writer.WriteUInt32(Level.packedVersion);
							byte[][] hwids = LocalHwid.GetHwids();
							writer.WriteUInt8((byte)hwids.Length);
							foreach (byte[] bytes in hwids)
							{
								writer.WriteBytes(bytes, 20);
							}
							writer.WriteBytes(TempSteamworksEconomy.econInfoHash, 20);
							writer.WriteSteamID(Provider.user);
						});
					}
				}
			}
		}

		/// <summary>
		/// Connect to server entry point on client.
		/// Requests workshop details for download prior to loading level.
		/// Once workshop is ready launch() is called.
		/// </summary>
		// Token: 0x0600368A RID: 13962 RVA: 0x0010C420 File Offset: 0x0010A620
		public static void connect(ServerConnectParameters parameters, SteamServerAdvertisement advertisement, List<PublishedFileId_t> expectedWorkshopItems)
		{
			if (Provider.isConnected)
			{
				return;
			}
			Provider._currentServerConnectParameters = parameters;
			Provider._currentServerAdvertisement = advertisement;
			Provider.isWhitelisted = false;
			Provider.isVacActive = false;
			Provider.isBattlEyeActive = false;
			Provider._isConnected = true;
			Provider._queuePosition = 0;
			Provider.resetChannels();
			if (Provider._currentServerAdvertisement != null)
			{
				Lobbies.LinkLobby(Provider._currentServerAdvertisement.ip, Provider._currentServerAdvertisement.queryPort);
				Provider._server = Provider._currentServerAdvertisement.steamID;
			}
			else
			{
				Lobbies.LinkLobby(parameters.address.value, parameters.queryPort);
				Provider._server = parameters.steamId;
			}
			Provider._serverPassword = parameters.password;
			Provider._serverPasswordHash = Hash.SHA1(parameters.password);
			Provider._isClient = true;
			Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
			Provider.pings = new float[4];
			Provider.lag((Provider._currentServerAdvertisement != null) ? ((float)Provider._currentServerAdvertisement.ping / 1000f) : 0f);
			Provider.isLoadingUGC = true;
			LoadingUI.updateScene();
			Provider.isWaitingForConnectResponse = false;
			Provider.isWaitingForWorkshopResponse = true;
			Provider.waitingForExpectedWorkshopItems = expectedWorkshopItems;
			Provider.isWaitingForAuthenticationResponse = false;
			List<SteamItemInstanceID_t> list = new List<SteamItemInstanceID_t>();
			if (Characters.active.packageShirt != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageShirt);
			}
			if (Characters.active.packagePants != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packagePants);
			}
			if (Characters.active.packageHat != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageHat);
			}
			if (Characters.active.packageBackpack != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageBackpack);
			}
			if (Characters.active.packageVest != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageVest);
			}
			if (Characters.active.packageMask != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageMask);
			}
			if (Characters.active.packageGlasses != 0UL)
			{
				list.Add((SteamItemInstanceID_t)Characters.active.packageGlasses);
			}
			for (int i = 0; i < Characters.packageSkins.Count; i++)
			{
				ulong num = Characters.packageSkins[i];
				if (num != 0UL)
				{
					list.Add((SteamItemInstanceID_t)num);
				}
			}
			if (list.Count > 0)
			{
				SteamInventory.GetItemsByID(out Provider.provider.economyService.wearingResult, list.ToArray(), (uint)list.Count);
			}
			Level.loading();
			Provider.ResetClientTransportFailure();
			SteamServerAdvertisement currentServerAdvertisement = Provider._currentServerAdvertisement;
			Provider.clientTransport = NetTransportFactory.CreateClientTransport((currentServerAdvertisement != null) ? currentServerAdvertisement.networkTransport : null);
			UnturnedLog.info("Initializing {0}", new object[]
			{
				Provider.clientTransport.GetType().Name
			});
			Provider.clientTransport.Initialize(new ClientTransportReady(Provider.onClientTransportReady), new ClientTransportFailure(Provider.onClientTransportFailure));
		}

		/// <summary>
		/// Callback once client transport is ready to send messages.
		/// </summary>
		// Token: 0x0600368B RID: 13963 RVA: 0x0010C6E4 File Offset: 0x0010A8E4
		private static void onClientTransportReady()
		{
			Provider.CachedWorkshopResponse cachedWorkshopResponse = null;
			foreach (Provider.CachedWorkshopResponse cachedWorkshopResponse2 in Provider.cachedWorkshopResponses)
			{
				if (cachedWorkshopResponse2.server == Provider.server && Time.realtimeSinceStartup - cachedWorkshopResponse2.realTime < 60f)
				{
					cachedWorkshopResponse = cachedWorkshopResponse2;
					break;
				}
			}
			if (cachedWorkshopResponse != null)
			{
				Provider.receiveWorkshopResponse(cachedWorkshopResponse);
				return;
			}
			NetMessages.SendMessageToServer(EServerMessage.GetWorkshopFiles, ENetReliability.Reliable, delegate(NetPakWriter writer)
			{
				writer.AlignToByte();
				for (int i = 0; i < 240; i++)
				{
					writer.WriteBits(0U, 32);
				}
				writer.WriteString("Hello!", 11);
			});
		}

		/// <summary>
		/// Callback when something goes wrong and client must disconnect.
		/// </summary>
		// Token: 0x0600368C RID: 13964 RVA: 0x0010C78C File Offset: 0x0010A98C
		private static void onClientTransportFailure(string message)
		{
			Provider.hasPendingClientTransportFailure = true;
			Provider.pendingClientTransportFailureMessage = message;
			if (Provider.canCurrentlyHandleClientTransportFailure)
			{
				Provider.TriggerDisconnectFromClientTransportFailure();
				return;
			}
			UnturnedLog.info("Deferring client transport failure because we can't currently handle it");
		}

		// Token: 0x0600368D RID: 13965 RVA: 0x0010C7B4 File Offset: 0x0010A9B4
		private static bool CompareClientAndServerWorkshopFileTimestamps()
		{
			if (Provider.provider.workshopService.serverPendingIDs == null)
			{
				return true;
			}
			foreach (PublishedFileId_t publishedFileId_t in Provider.provider.workshopService.serverPendingIDs)
			{
				Provider.ServerRequiredWorkshopFile serverRequiredWorkshopFile;
				ulong num;
				string text;
				uint time;
				if (!Provider.currentServerWorkshopResponse.FindRequiredFile(publishedFileId_t.m_PublishedFileId, out serverRequiredWorkshopFile))
				{
					UnturnedLog.error(string.Format("Server workshop files response missing details for file: {0}", publishedFileId_t));
				}
				else if (serverRequiredWorkshopFile.timestamp.Year < 2000)
				{
					UnturnedLog.info(string.Format("Skipping timestamp comparison for server workshop file {0} because timestamp is invalid ({1})", publishedFileId_t, serverRequiredWorkshopFile.timestamp.ToLocalTime()));
				}
				else if (!SteamUGC.GetItemInstallInfo(publishedFileId_t, out num, out text, 1024U, out time))
				{
					UnturnedLog.info(string.Format("Skipping timestamp comparison for server workshop file {0} because item install info is missing", publishedFileId_t));
				}
				else
				{
					DateTime dateTime = DateTimeEx.FromUtcUnixTimeSeconds(time);
					if (!(dateTime == serverRequiredWorkshopFile.timestamp))
					{
						CachedUGCDetails cachedUGCDetails;
						bool cachedDetails = TempSteamworksWorkshop.getCachedDetails(publishedFileId_t, out cachedUGCDetails);
						string str;
						if (cachedDetails)
						{
							str = cachedUGCDetails.GetTitle();
						}
						else
						{
							str = string.Format("Unknown File ID {0}", publishedFileId_t);
						}
						Provider._connectionFailureInfo = ESteamConnectionFailureInfo.CUSTOM;
						string text2;
						if (serverRequiredWorkshopFile.timestamp > dateTime)
						{
							text2 = "Server is running a newer version of the \"" + str + "\" workshop file.";
						}
						else
						{
							text2 = "Server is running an older version of the \"" + str + "\" workshop file.";
						}
						if (cachedDetails)
						{
							DateTime dateTime2 = DateTimeEx.FromUtcUnixTimeSeconds(cachedUGCDetails.updateTimestamp);
							if (dateTime == dateTime2)
							{
								text2 += "\nYour installed copy of the file matches the most recent version on Steam.";
								text2 += string.Format("\nLocal and Steam timestamp: {0} Server timestamp: {1}", dateTime.ToLocalTime(), serverRequiredWorkshopFile.timestamp.ToLocalTime());
							}
							else if (serverRequiredWorkshopFile.timestamp == dateTime2)
							{
								text2 += "\nThe server's installed copy of the file matches the most recent version on Steam.";
								text2 += string.Format("\nLocal timestamp: {0} Server and Steam timestamp: {1}", dateTime.ToLocalTime(), serverRequiredWorkshopFile.timestamp.ToLocalTime());
							}
							else
							{
								text2 += string.Format("\nLocal timestamp: {0} Server timestamp: {1} Steam timestamp: {2}", dateTime.ToLocalTime(), serverRequiredWorkshopFile.timestamp.ToLocalTime(), dateTime2);
							}
						}
						else
						{
							text2 += string.Format("\nLocal timestamp: {0} Server timestamp: {1}", dateTime.ToLocalTime(), serverRequiredWorkshopFile.timestamp.ToLocalTime());
						}
						Provider._connectionFailureReason = text2;
						Provider.RequestDisconnect(string.Format("Loaded workshop file timestamp mismatch (File ID: {0} Local timestamp: {1} Server timestamp: {2})", publishedFileId_t, dateTime.ToLocalTime(), serverRequiredWorkshopFile.timestamp.ToLocalTime()));
						return false;
					}
					UnturnedLog.info(string.Format("Workshop file {0} timestamp matches between client and server ({1})", publishedFileId_t, dateTime));
				}
			}
			return true;
		}

		/// <summary>
		/// Multiplayer load level entry point on client.
		/// Called once workshop downloads are finished, or we know the server is not using workshop.
		/// Once level is loaded the connect packet is sent to the server.
		/// </summary>
		// Token: 0x0600368E RID: 13966 RVA: 0x0010CAC4 File Offset: 0x0010ACC4
		public static void launch()
		{
			LevelInfo level = Level.getLevel(Provider.map);
			if (level == null)
			{
				Provider._connectionFailureInfo = ESteamConnectionFailureInfo.MAP;
				Provider.RequestDisconnect("could not find level \"" + Provider.map + "\"");
				return;
			}
			if (!Provider.CompareClientAndServerWorkshopFileTimestamps())
			{
				return;
			}
			if (Provider.hasPendingClientTransportFailure)
			{
				UnturnedLog.info("Now able to handle client transport failure that occurred during workshop file download/install/load");
				Provider.TriggerDisconnectFromClientTransportFailure();
				return;
			}
			Assets.ApplyServerAssetMapping(level, Provider.provider.workshopService.serverPendingIDs);
			Provider.canCurrentlyHandleClientTransportFailure = false;
			UnturnedLog.info("Loading server level ({0})", new object[]
			{
				Provider.map
			});
			Level.load(level, false);
			Provider.loadGameMode();
		}

		// Token: 0x0600368F RID: 13967 RVA: 0x0010CB60 File Offset: 0x0010AD60
		private static void loadGameMode()
		{
			LevelAsset asset = Level.getAsset();
			if (asset == null)
			{
				Provider.gameMode = new SurvivalGameMode();
				return;
			}
			Type type = asset.defaultGameMode.type;
			if (type == null)
			{
				Provider.gameMode = new SurvivalGameMode();
				return;
			}
			Provider.gameMode = (Activator.CreateInstance(type) as GameMode);
			if (Provider.gameMode == null)
			{
				Provider.gameMode = new SurvivalGameMode();
			}
		}

		// Token: 0x06003690 RID: 13968 RVA: 0x0010CBC2 File Offset: 0x0010ADC2
		private static void unloadGameMode()
		{
			Provider.gameMode = null;
		}

		// Token: 0x06003691 RID: 13969 RVA: 0x0010CBCC File Offset: 0x0010ADCC
		public static void singleplayer(EGameMode singleplayerMode, bool singleplayerCheats)
		{
			Provider._isConnected = true;
			Provider.resetChannels();
			Dedicator.serverVisibility = ESteamServerVisibility.LAN;
			Dedicator.serverID = "Singleplayer_" + Characters.selected.ToString();
			Commander.init();
			Provider.maxPlayers = 1;
			Provider.queueSize = 8;
			Provider.serverName = "Singleplayer #" + ((int)(Characters.selected + 1)).ToString();
			Provider.serverPassword = "Singleplayer";
			Provider.isVacActive = false;
			Provider.isBattlEyeActive = false;
			Provider.ip = 0U;
			Provider.port = 25000;
			Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
			Provider.pings = new float[4];
			Provider.isPvP = true;
			Provider.isWhitelisted = false;
			Provider.hideAdmins = false;
			Provider.hasCheats = singleplayerCheats;
			Provider.filterName = false;
			Provider.mode = singleplayerMode;
			Provider.isGold = false;
			Provider.gameMode = null;
			Provider.cameraMode = ECameraMode.BOTH;
			if (singleplayerMode != EGameMode.TUTORIAL)
			{
				PlayerInventory.skillsets = PlayerInventory.SKILLSETS_CLIENT;
			}
			Provider.lag(0f);
			SteamWhitelist.load();
			SteamBlacklist.load();
			SteamAdminlist.load();
			Provider._currentServerAdvertisement = null;
			Provider._configData = ConfigData.CreateDefault(true);
			if (ServerSavedata.fileExists("/Config.json"))
			{
				try
				{
					ServerSavedata.populateJSON("/Config.json", Provider._configData);
				}
				catch (Exception e)
				{
					UnturnedLog.error("Exception while parsing singleplayer config:");
					UnturnedLog.exception(e);
				}
			}
			Provider._modeConfigData = Provider._configData.getModeConfig(Provider.mode);
			if (Provider._modeConfigData == null)
			{
				Provider._modeConfigData = new ModeConfigData(Provider.mode);
				Provider._modeConfigData.InitSingleplayerDefaults();
			}
			Provider.authorityHoliday = (Provider._modeConfigData.Gameplay.Allow_Holidays ? HolidayUtil.BackendGetActiveHoliday() : ENPCHoliday.NONE);
			Provider._isServer = true;
			Provider._isClient = true;
			Provider.time = SteamUtils.GetServerRealTime();
			Level.load(Level.getLevel(Provider.map), true);
			Provider.loadGameMode();
			Provider.applyLevelModeConfigOverrides();
			Provider._server = Provider.user;
			Provider._client = Provider._server;
			Provider._clientHash = Hash.SHA1(Provider.client);
			Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
			Provider.broadcastServerHosted();
		}

		// Token: 0x06003692 RID: 13970 RVA: 0x0010CDD0 File Offset: 0x0010AFD0
		public static void host()
		{
			Provider._isConnected = true;
			Provider.resetChannels();
			Provider.openGameServer();
			Provider._isServer = true;
			Provider.broadcastServerHosted();
		}

		/// <summary>
		/// Event for plugins prior to kicking players during shutdown.
		/// </summary>
		// Token: 0x140000D3 RID: 211
		// (add) Token: 0x06003693 RID: 13971 RVA: 0x0010CDF0 File Offset: 0x0010AFF0
		// (remove) Token: 0x06003694 RID: 13972 RVA: 0x0010CE24 File Offset: 0x0010B024
		public static event Provider.CommenceShutdownHandler onCommenceShutdown;

		// Token: 0x06003695 RID: 13973 RVA: 0x0010CE58 File Offset: 0x0010B058
		private static void broadcastCommenceShutdown()
		{
			try
			{
				Provider.CommenceShutdownHandler commenceShutdownHandler = Provider.onCommenceShutdown;
				if (commenceShutdownHandler != null)
				{
					commenceShutdownHandler();
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onCommenceShutdown:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x06003696 RID: 13974 RVA: 0x0010CE98 File Offset: 0x0010B098
		public static void shutdown()
		{
			Provider.shutdown(0);
		}

		// Token: 0x06003697 RID: 13975 RVA: 0x0010CEA0 File Offset: 0x0010B0A0
		public static void shutdown(int timer)
		{
			Provider.shutdown(timer, string.Empty);
		}

		// Token: 0x06003698 RID: 13976 RVA: 0x0010CEAD File Offset: 0x0010B0AD
		public static void shutdown(int timer, string explanation)
		{
			Provider.countShutdownTimer = timer;
			Provider.lastTimerMessage = Time.realtimeSinceStartup;
			Provider.shutdownMessage = explanation;
			UnturnedLog.info(string.Format("Set server shutdown timer to {0}s (client message: {1})", timer, explanation));
		}

		// Token: 0x17000998 RID: 2456
		// (get) Token: 0x06003699 RID: 13977 RVA: 0x0010CEDB File Offset: 0x0010B0DB
		internal static bool IsBattlEyeEnabled
		{
			get
			{
				return Provider.configData != null && Provider.configData.Server.BattlEye_Secure && !Dedicator.offlineOnly;
			}
		}

		// Token: 0x0600369A RID: 13978 RVA: 0x0010CF04 File Offset: 0x0010B104
		public static void RequestDisconnect(string reason)
		{
			UnturnedLog.info("Disconnecting: " + reason);
			Provider.disconnect();
		}

		/// <summary>
		/// Client should call RequestDisconnect instead to ensure all disconnects have a logged reason.
		/// </summary>
		// Token: 0x0600369B RID: 13979 RVA: 0x0010CF1C File Offset: 0x0010B11C
		public static void disconnect()
		{
			if (!Dedicator.IsDedicatedServer && Player.player != null && Player.player.channel != null && Player.player.channel.owner != null)
			{
				Player.player.channel.owner.commitModifiedDynamicProps();
			}
			if (Provider.isServer)
			{
				if (Provider.isBattlEyeActive && Provider.battlEyeServerHandle != IntPtr.Zero)
				{
					if (Provider.battlEyeServerRunData != null && Provider.battlEyeServerRunData.pfnExit != null)
					{
						UnturnedLog.info("Shutting down BattlEye server");
						bool flag = Provider.battlEyeServerRunData.pfnExit();
						UnturnedLog.info("BattlEye server shutdown result: {0}", new object[]
						{
							flag
						});
					}
					BEServer.FreeLibrary(Provider.battlEyeServerHandle);
					Provider.battlEyeServerHandle = IntPtr.Zero;
				}
				if (Provider.serverTransport != null)
				{
					Provider.serverTransport.TearDown();
				}
				if (Dedicator.IsDedicatedServer)
				{
					Provider.closeGameServer();
				}
				else
				{
					Provider.broadcastServerShutdown();
				}
				if (Provider.isClient)
				{
					Provider._client = Provider.user;
					Provider._clientHash = Hash.SHA1(Provider.client);
				}
				Provider._isServer = false;
				Provider._isClient = false;
			}
			else if (Provider.isClient)
			{
				if (Provider.battlEyeClientHandle != IntPtr.Zero)
				{
					if (Provider.battlEyeClientRunData != null && Provider.battlEyeClientRunData.pfnExit != null)
					{
						UnturnedLog.info("Shutting down BattlEye client");
						bool flag2 = Provider.battlEyeClientRunData.pfnExit();
						UnturnedLog.info("BattlEye client shutdown result: {0}", new object[]
						{
							flag2
						});
					}
					BEClient.FreeLibrary(Provider.battlEyeClientHandle);
					Provider.battlEyeClientHandle = IntPtr.Zero;
				}
				NetMessages.SendMessageToServer(EServerMessage.GracefullyDisconnect, ENetReliability.Reliable, delegate(NetPakWriter writer)
				{
				});
				Provider.clientTransport.TearDown();
				SteamFriends.SetRichPresence("connect", "");
				Lobbies.leaveLobby();
				Provider.closeTicket();
				SteamUser.AdvertiseGame(CSteamID.Nil, 0U, 0);
				Provider._server = default(CSteamID);
				Provider._isServer = false;
				Provider._isClient = false;
			}
			Provider.ClientDisconnected clientDisconnected = Provider.onClientDisconnected;
			if (clientDisconnected != null)
			{
				clientDisconnected();
			}
			if (!Provider.isApplicationQuitting)
			{
				Provider.authorityHoliday = HolidayUtil.BackendGetActiveHoliday();
				Level.exit();
			}
			Assets.ClearServerAssetMapping();
			Provider.unloadGameMode();
			Provider._isConnected = false;
			Provider.isWaitingForConnectResponse = false;
			Provider.isWaitingForWorkshopResponse = false;
			Provider.isLoadingUGC = false;
			Provider.isWaitingForAuthenticationResponse = false;
			Provider.isLoadingInventory = true;
			UnturnedLog.info("Disconnected");
		}

		// Token: 0x0600369C RID: 13980 RVA: 0x0010D187 File Offset: 0x0010B387
		[Obsolete]
		public static void sendGUIDTable(SteamPending player)
		{
			Provider.accept(player);
		}

		// Token: 0x0600369D RID: 13981 RVA: 0x0010D190 File Offset: 0x0010B390
		private static bool initializeBattlEyeServer()
		{
			string text = ReadWrite.PATH + "/BattlEye/BEServer_x64.dll";
			if (!File.Exists(text))
			{
				text = ReadWrite.PATH + "/BattlEye/BEServer.dll";
			}
			if (!File.Exists(text))
			{
				UnturnedLog.error("Missing BattlEye server library! (" + text + ")");
				return false;
			}
			UnturnedLog.info("Loading BattlEye server library from: " + text);
			bool result;
			try
			{
				Provider.battlEyeServerHandle = BEServer.LoadLibraryW(text);
				if (Provider.battlEyeServerHandle != IntPtr.Zero)
				{
					BEServer.BEServerInitFn beserverInitFn = Marshal.GetDelegateForFunctionPointer(BEServer.GetProcAddress(Provider.battlEyeServerHandle, "Init"), typeof(BEServer.BEServerInitFn)) as BEServer.BEServerInitFn;
					if (beserverInitFn != null)
					{
						Provider.battlEyeServerInitData = new BEServer.BESV_GAME_DATA();
						Provider.battlEyeServerInitData.pstrGameVersion = Provider.APP_NAME + " " + Provider.APP_VERSION;
						Provider.battlEyeServerInitData.pfnPrintMessage = new BEServer.BESV_GAME_DATA.PrintMessageFn(Provider.battlEyeServerPrintMessage);
						Provider.battlEyeServerInitData.pfnKickPlayer = new BEServer.BESV_GAME_DATA.KickPlayerFn(Provider.battlEyeServerKickPlayer);
						Provider.battlEyeServerInitData.pfnSendPacket = new BEServer.BESV_GAME_DATA.SendPacketFn(Provider.battlEyeServerSendPacket);
						Provider.battlEyeServerRunData = new BEServer.BESV_BE_DATA();
						if (beserverInitFn(0, Provider.battlEyeServerInitData, Provider.battlEyeServerRunData))
						{
							result = true;
						}
						else
						{
							BEServer.FreeLibrary(Provider.battlEyeServerHandle);
							Provider.battlEyeServerHandle = IntPtr.Zero;
							UnturnedLog.error("Failed to call BattlEye server init!");
							result = false;
						}
					}
					else
					{
						BEServer.FreeLibrary(Provider.battlEyeServerHandle);
						Provider.battlEyeServerHandle = IntPtr.Zero;
						UnturnedLog.error("Failed to get BattlEye server init delegate!");
						result = false;
					}
				}
				else
				{
					UnturnedLog.error("Failed to load BattlEye server library!");
					result = false;
				}
			}
			catch (Exception e)
			{
				UnturnedLog.error("Unhandled exception when loading BattlEye server library!");
				UnturnedLog.exception(e);
				result = false;
			}
			return result;
		}

		/// <summary>
		/// Internet server callback when backend is ready.
		/// </summary>
		// Token: 0x0600369E RID: 13982 RVA: 0x0010D34C File Offset: 0x0010B54C
		private static void handleServerReady()
		{
			if (Provider.isServerConnectedToSteam)
			{
				return;
			}
			Provider.isServerConnectedToSteam = true;
			CommandWindow.Log("Steam servers ready!");
			Provider.initializeDedicatedUGC();
		}

		// Token: 0x0600369F RID: 13983 RVA: 0x0010D36C File Offset: 0x0010B56C
		private static void initializeDedicatedUGC()
		{
			WorkshopDownloadConfig orLoad = WorkshopDownloadConfig.getOrLoad();
			DedicatedUGC.initialize();
			if (Assets.shouldLoadAnyAssets)
			{
				foreach (ulong id in orLoad.File_IDs)
				{
					DedicatedUGC.registerItemInstallation(id);
				}
			}
			DedicatedUGC.installed += Provider.onDedicatedUGCInstalled;
			DedicatedUGC.beginInstallingItems(Dedicator.offlineOnly);
		}

		// Token: 0x060036A0 RID: 13984 RVA: 0x0010D3F4 File Offset: 0x0010B5F4
		public static string getModeTagAbbreviation(EGameMode gm)
		{
			switch (gm)
			{
			case EGameMode.EASY:
				return "EZY";
			case EGameMode.NORMAL:
				return "NRM";
			case EGameMode.HARD:
				return "HRD";
			default:
				return null;
			}
		}

		// Token: 0x060036A1 RID: 13985 RVA: 0x0010D41D File Offset: 0x0010B61D
		public static string getCameraModeTagAbbreviation(ECameraMode cm)
		{
			switch (cm)
			{
			case ECameraMode.FIRST:
				return "1Pp";
			case ECameraMode.THIRD:
				return "3Pp";
			case ECameraMode.BOTH:
				return "2Pp";
			case ECameraMode.VEHICLE:
				return "4Pp";
			default:
				return null;
			}
		}

		// Token: 0x060036A2 RID: 13986 RVA: 0x0010D450 File Offset: 0x0010B650
		public static string GetMonetizationTagAbbreviation(EServerMonetizationTag monetization)
		{
			switch (monetization)
			{
			case EServerMonetizationTag.None:
				return "MTXn";
			case EServerMonetizationTag.NonGameplay:
				return "MTXy";
			case EServerMonetizationTag.Monetized:
				return "MTXg";
			default:
				return null;
			}
		}

		/// <summary>
		/// If missing map is a curated map then log information about how to install it.
		/// </summary>
		// Token: 0x060036A3 RID: 13987 RVA: 0x0010D47C File Offset: 0x0010B67C
		private static void maybeLogCuratedMapFallback(string attemptedMap)
		{
			if (Provider.statusData == null || Provider.statusData.Maps == null || Provider.statusData.Maps.Curated_Map_Links == null)
			{
				return;
			}
			foreach (CuratedMapLink curatedMapLink in Provider.statusData.Maps.Curated_Map_Links)
			{
				if (curatedMapLink.Name.Equals(attemptedMap, StringComparison.InvariantCultureIgnoreCase))
				{
					CommandWindow.LogWarningFormat("Attempting to load curated map '{0}'? Include its workshop file ID ({1}) in the WorkshopDownloadConfig.json File_IDs array.", new object[]
					{
						curatedMapLink.Name,
						curatedMapLink.Workshop_File_Id
					});
					break;
				}
			}
		}

		// Token: 0x060036A4 RID: 13988 RVA: 0x0010D530 File Offset: 0x0010B730
		private static void onDedicatedUGCInstalled()
		{
			if (Provider.isDedicatedUGCInstalled)
			{
				return;
			}
			Provider.isDedicatedUGCInstalled = true;
			Provider.apiWarningMessageHook = new SteamAPIWarningMessageHook_t(Provider.onAPIWarningMessage);
			SteamGameServerUtils.SetWarningMessageHook(Provider.apiWarningMessageHook);
			Provider.time = SteamGameServerUtils.GetServerRealTime();
			LevelInfo level = Level.getLevel(Provider.map);
			if (level == null)
			{
				string text = Provider.map;
				Provider.maybeLogCuratedMapFallback(text);
				Provider.map = "PEI";
				CommandWindow.LogError(Provider.localization.format("Map_Missing", text, Provider.map));
				level = Level.getLevel(Provider.map);
				if (level == null)
				{
					CommandWindow.LogError("Fatal error: unable to load fallback map");
				}
			}
			if (level != null)
			{
				Provider.map = level.name;
			}
			List<PublishedFileId_t> list = null;
			if (Provider._serverWorkshopFileIDs != null)
			{
				list = new List<PublishedFileId_t>(Provider._serverWorkshopFileIDs.Count);
				foreach (ulong value in Provider._serverWorkshopFileIDs)
				{
					list.Add(new PublishedFileId_t(value));
				}
			}
			Assets.ApplyServerAssetMapping(level, list);
			Level.load(level, true);
			Provider.loadGameMode();
			Provider.applyLevelModeConfigOverrides();
			SteamGameServer.SetMaxPlayerCount((int)Provider.maxPlayers);
			SteamGameServer.SetServerName(Provider.serverName);
			SteamGameServer.SetPasswordProtected(Provider.serverPassword != "");
			SteamGameServer.SetMapName(Provider.map);
			if (Dedicator.IsDedicatedServer)
			{
				if (!ReadWrite.folderExists("/Bundles/Workshop/Content", true))
				{
					ReadWrite.createFolder("/Bundles/Workshop/Content", true);
				}
				string text2 = "/Bundles/Workshop/Content";
				string[] folders = ReadWrite.getFolders(text2);
				for (int i = 0; i < folders.Length; i++)
				{
					string text3 = ReadWrite.folderName(folders[i]);
					ulong id;
					if (ulong.TryParse(text3, NumberStyles.Any, CultureInfo.InvariantCulture, out id))
					{
						Provider.registerServerUsingWorkshopFileId(id);
						CommandWindow.Log("Recommended to add workshop item " + id.ToString() + " to WorkshopDownloadConfig.json and remove it from " + text2);
					}
					else
					{
						CommandWindow.LogWarning("Invalid workshop item '" + text3 + "' in " + text2);
					}
				}
				string text4 = ServerSavedata.directory + "/" + Provider.serverID + "/Workshop/Content";
				if (!ReadWrite.folderExists(text4, true))
				{
					ReadWrite.createFolder(text4, true);
				}
				string[] folders2 = ReadWrite.getFolders(text4);
				for (int j = 0; j < folders2.Length; j++)
				{
					string text5 = ReadWrite.folderName(folders2[j]);
					ulong id2;
					if (ulong.TryParse(text5, NumberStyles.Any, CultureInfo.InvariantCulture, out id2))
					{
						Provider.registerServerUsingWorkshopFileId(id2);
						CommandWindow.Log("Recommended to add workshop item " + id2.ToString() + " to WorkshopDownloadConfig.json and remove it from " + text4);
					}
					else
					{
						CommandWindow.LogWarning("Invalid workshop item '" + text5 + "' in " + text4);
					}
				}
				ulong id3;
				if (ulong.TryParse(new DirectoryInfo(Level.info.path).Parent.Name, NumberStyles.Any, CultureInfo.InvariantCulture, out id3))
				{
					Provider.registerServerUsingWorkshopFileId(id3);
				}
				SteamGameServer.SetGameData(((Provider.serverPassword != "") ? "PASS" : "SSAP") + "," + (Provider.configData.Server.VAC_Secure ? "VAC_ON" : "VAC_OFF") + ",GAME_VERSION_" + VersionUtils.binaryToHexadecimal(Provider.APP_VERSION_PACKED) + ",MAP_VERSION_" + VersionUtils.binaryToHexadecimal(Level.packedVersion));
				SteamGameServer.SetKeyValue("GameVersion", Provider.APP_VERSION);
				int num = 128;
				string text6 = string.Concat(new string[]
				{
					Provider.isPvP ? "PVP" : "PVE",
					",",
					Provider.hasCheats ? "CHy" : "CHn",
					",",
					Provider.getModeTagAbbreviation(Provider.mode),
					",",
					Provider.getCameraModeTagAbbreviation(Provider.cameraMode),
					",",
					(Provider.getServerWorkshopFileIDs().Count > 0) ? "WSy" : "WSn",
					",",
					Provider.isGold ? "GLD" : "F2P"
				});
				text6 = text6 + "," + (Provider.isBattlEyeActive ? "BEy" : "BEn");
				if (!Provider.hasSetIsBattlEyeActive)
				{
					CommandWindow.LogError("Order of things is messed up! isBattlEyeActive should have been set before advertising game server.");
				}
				string monetizationTagAbbreviation = Provider.GetMonetizationTagAbbreviation(Provider.configData.Browser.Monetization);
				if (!string.IsNullOrEmpty(monetizationTagAbbreviation))
				{
					text6 = text6 + "," + monetizationTagAbbreviation;
				}
				if (!string.IsNullOrEmpty(Provider.configData.Browser.Thumbnail))
				{
					text6 = text6 + ",<tn>" + Provider.configData.Browser.Thumbnail + "</tn>";
				}
				text6 += string.Format(",<net>{0}</net>", NetTransportFactory.GetTag(Provider.serverTransport));
				string pluginFrameworkTag = SteamPluginAdvertising.Get().PluginFrameworkTag;
				if (!string.IsNullOrEmpty(pluginFrameworkTag))
				{
					text6 += string.Format(",<pf>{0}</pf>", pluginFrameworkTag);
				}
				if (text6.Length > num)
				{
					CommandWindow.LogWarning("Server browser thumbnail URL is " + (text6.Length - num).ToString() + " characters over budget!");
					CommandWindow.LogWarning("Server will not list properly until this URL is adjusted!");
				}
				SteamGameServer.SetGameTags(text6);
				int num2 = 64;
				if (Provider.configData.Browser.Desc_Server_List.Length > num2)
				{
					CommandWindow.LogWarning("Server browser description is " + (Provider.configData.Browser.Desc_Server_List.Length - num2).ToString() + " characters over budget!");
				}
				SteamGameServer.SetGameDescription(Provider.configData.Browser.Desc_Server_List);
				SteamGameServer.SetKeyValue("Browser_Icon", Provider.configData.Browser.Icon);
				SteamGameServer.SetKeyValue("Browser_Desc_Hint", Provider.configData.Browser.Desc_Hint);
				Provider.AdvertiseFullDescription(Provider.configData.Browser.Desc_Full);
				if (Provider.getServerWorkshopFileIDs().Count > 0)
				{
					string text7 = string.Empty;
					for (int k = 0; k < Provider.getServerWorkshopFileIDs().Count; k++)
					{
						if (text7.Length > 0)
						{
							text7 += ",";
						}
						text7 += Provider.getServerWorkshopFileIDs()[k].ToString();
					}
					int num3 = (text7.Length - 1) / 127 + 1;
					int num4 = 0;
					SteamGameServer.SetKeyValue("Mod_Count", num3.ToString());
					for (int l = 0; l < text7.Length; l += 127)
					{
						int num5 = 127;
						if (l + num5 > text7.Length)
						{
							num5 = text7.Length - l;
						}
						string pValue = text7.Substring(l, num5);
						SteamGameServer.SetKeyValue("Mod_" + num4.ToString(), pValue);
						num4++;
					}
				}
				if (Provider.configData.Browser.Links != null && Provider.configData.Browser.Links.Length != 0)
				{
					SteamGameServer.SetKeyValue("Custom_Links_Count", Provider.configData.Browser.Links.Length.ToString());
					for (int m = 0; m < Provider.configData.Browser.Links.Length; m++)
					{
						BrowserConfigData.Link link = Provider.configData.Browser.Links[m];
						string pValue2;
						string pValue3;
						if (!ConvertEx.TryEncodeUtf8StringAsBase64(link.Message, out pValue2))
						{
							UnturnedLog.error("Unable to encode lobby link message as Base64: \"" + link.Message + "\"");
						}
						else if (!ConvertEx.TryEncodeUtf8StringAsBase64(link.Url, out pValue3))
						{
							UnturnedLog.error("Unable to encode lobby link url as Base64: \"" + link.Url + "\"");
						}
						else
						{
							SteamGameServer.SetKeyValue("Custom_Link_Message_" + m.ToString(), pValue2);
							SteamGameServer.SetKeyValue("Custom_Link_Url_" + m.ToString(), pValue3);
						}
					}
				}
				Provider.AdvertiseConfig();
				SteamPluginAdvertising.Get().NotifyGameServerReady();
			}
			Provider.dswUpdateMonitor = DedicatedWorkshopUpdateMonitorFactory.createForLevel(level);
			Provider._server = SteamGameServer.GetSteamID();
			Provider._client = Provider._server;
			Provider._clientHash = Hash.SHA1(Provider.client);
			if (Dedicator.IsDedicatedServer)
			{
				Provider._clientName = Provider.localization.format("Console");
				Provider.autoShutdownManager = Provider.steam.gameObject.AddComponent<BuiltinAutoShutdown>();
				uint value2;
				SteamGameServer.GetPublicIP().TryGetIPv4Address(out value2);
				EHostBanFlags ehostBanFlags = HostBansManager.Get().MatchBasicDetails(new IPv4Address(value2), Provider.port, Provider.serverName, Provider._server.m_SteamID);
				ehostBanFlags |= HostBansManager.Get().MatchExtendedDetails(Provider.configData.Browser.Desc_Server_List, Provider.configData.Browser.Thumbnail);
				if ((ehostBanFlags & EHostBanFlags.RecommendHostCheckWarningsList) != EHostBanFlags.None)
				{
					CommandWindow.LogWarning("It appears this server has received a warning.");
					CommandWindow.LogWarning("Checking the Unturned Server Standing page is recommended:");
					CommandWindow.LogWarning("https://smartlydressedgames.com/UnturnedHostBans/index.html");
				}
				if (ehostBanFlags.HasFlag(EHostBanFlags.Blocked))
				{
					Provider.shutdown();
				}
			}
			Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
		}

		/// <summary>
		/// Set key/value tags on Steam server advertisement so that client can display text in browser.
		/// </summary>
		// Token: 0x060036A5 RID: 13989 RVA: 0x0010DE30 File Offset: 0x0010C030
		private static void AdvertiseFullDescription(string message)
		{
			if (string.IsNullOrEmpty(message))
			{
				return;
			}
			string text;
			if (!ConvertEx.TryEncodeUtf8StringAsBase64(message, out text))
			{
				UnturnedLog.error("Unable to encode server browser description to Base64");
				return;
			}
			if (string.IsNullOrEmpty(text))
			{
				UnturnedLog.error("Base64 server browser description was empty");
				return;
			}
			int num = (text.Length - 1) / 127 + 1;
			int num2 = 0;
			SteamGameServer.SetKeyValue("Browser_Desc_Full_Count", num.ToString());
			for (int i = 0; i < text.Length; i += 127)
			{
				int num3 = 127;
				if (i + num3 > text.Length)
				{
					num3 = text.Length - i;
				}
				string pValue = text.Substring(i, num3);
				SteamGameServer.SetKeyValue("Browser_Desc_Full_Line_" + num2.ToString(), pValue);
				num2++;
			}
		}

		/// <summary>
		/// Set key/value tags on Steam server advertisement so that client can display server config in browser.
		/// </summary>
		// Token: 0x060036A6 RID: 13990 RVA: 0x0010DEE4 File Offset: 0x0010C0E4
		private static void AdvertiseConfig()
		{
			ModeConfigData modeConfig = ConfigData.CreateDefault(false).getModeConfig(Provider.mode);
			if (modeConfig == null)
			{
				CommandWindow.LogError("Unable to compare default for advertise config");
				return;
			}
			int num = 0;
			foreach (FieldInfo fieldInfo in Provider.modeConfigData.GetType().GetFields())
			{
				object value = fieldInfo.GetValue(Provider.modeConfigData);
				object value2 = fieldInfo.GetValue(modeConfig);
				foreach (FieldInfo fieldInfo2 in value.GetType().GetFields())
				{
					object value3 = fieldInfo2.GetValue(value);
					object value4 = fieldInfo2.GetValue(value2);
					string text = null;
					Type fieldType = fieldInfo2.FieldType;
					if (fieldType == typeof(bool))
					{
						bool flag = (bool)value3;
						bool flag2 = (bool)value4;
						if (flag != flag2)
						{
							text = string.Concat(new string[]
							{
								fieldInfo.Name,
								".",
								fieldInfo2.Name,
								"=",
								flag ? "T" : "F"
							});
						}
					}
					else if (fieldType == typeof(float))
					{
						float a = (float)value3;
						float b = (float)value4;
						if (!MathfEx.IsNearlyEqual(a, b, 0.0001f))
						{
							text = string.Concat(new string[]
							{
								fieldInfo.Name,
								".",
								fieldInfo2.Name,
								"=",
								a.ToString(CultureInfo.InvariantCulture)
							});
						}
					}
					else if (fieldType == typeof(uint))
					{
						uint num2 = (uint)value3;
						uint num3 = (uint)value4;
						if (num2 != num3)
						{
							text = string.Concat(new string[]
							{
								fieldInfo.Name,
								".",
								fieldInfo2.Name,
								"=",
								num2.ToString(CultureInfo.InvariantCulture)
							});
						}
					}
					else
					{
						CommandWindow.LogErrorFormat("Unable to advertise config type: {0}", new object[]
						{
							fieldType
						});
					}
					if (!string.IsNullOrEmpty(text))
					{
						string pKey = "Cfg_" + num.ToString(CultureInfo.InvariantCulture);
						num++;
						SteamGameServer.SetKeyValue(pKey, text);
					}
				}
			}
			SteamGameServer.SetKeyValue("Cfg_Count", num.ToString(CultureInfo.InvariantCulture));
		}

		/// <summary>
		/// Primarily kept for backwards compatibility with plugins. Some RPCs that reply to sender also use this but
		/// should be tidied up.
		/// </summary>
		// Token: 0x060036A7 RID: 13991 RVA: 0x0010E15C File Offset: 0x0010C35C
		[Obsolete]
		public static void send(CSteamID steamID, ESteamPacket type, byte[] packet, int size, int channel)
		{
			ITransportConnection transportConnection = Provider.findTransportConnection(steamID);
			if (transportConnection != null)
			{
				Provider.sendToClient(transportConnection, type, packet, size);
			}
		}

		/// <summary>
		/// Hack to deal with the oversight of reordering the ESteamPacket enum during net messaging rewrite causing
		/// older plugins to send wrong packet type.
		/// </summary>
		// Token: 0x060036A8 RID: 13992 RVA: 0x0010E17C File Offset: 0x0010C37C
		[Obsolete]
		private static bool remapSteamPacketType(ref ESteamPacket type)
		{
			ESteamPacket esteamPacket = type;
			if (esteamPacket == ESteamPacket.KICKED)
			{
				type = ESteamPacket.UPDATE_RELIABLE_BUFFER;
				return true;
			}
			if (esteamPacket != ESteamPacket.CONNECTED)
			{
				return false;
			}
			type = ESteamPacket.UPDATE_UNRELIABLE_BUFFER;
			return true;
		}

		/// <summary>
		/// Send to a connected client.
		/// </summary>
		// Token: 0x060036A9 RID: 13993 RVA: 0x0010E1A4 File Offset: 0x0010C3A4
		[Obsolete]
		public static void sendToClient(ITransportConnection transportConnection, ESteamPacket type, byte[] packet, int size)
		{
			if (size < 1)
			{
				throw new ArgumentOutOfRangeException("size");
			}
			if (transportConnection == null)
			{
				throw new ArgumentNullException("transportConnection");
			}
			if (!Provider.isConnected)
			{
				return;
			}
			if (!Provider.isServer)
			{
				throw new NotSupportedException("Sending to client while not running as server");
			}
			if (Provider.remapSteamPacketType(ref type))
			{
				packet[0] = (byte)type;
			}
			Provider._bytesSent += (uint)size;
			Provider._packetsSent += 1U;
			ENetReliability reliability;
			if (Provider.isUnreliable(type))
			{
				reliability = ENetReliability.Unreliable;
			}
			else
			{
				reliability = ENetReliability.Reliable;
			}
			transportConnection.Send(packet, (long)size, reliability);
		}

		/// <summary>
		/// Hacked-together initial implementation to refuse network messages from specific players.
		/// On PC some cheats send garbage packets in which case those clients should be blocked.
		/// </summary>
		// Token: 0x060036AA RID: 13994 RVA: 0x0010E226 File Offset: 0x0010C426
		public static bool shouldNetIgnoreSteamId(CSteamID id)
		{
			return Provider.netIgnoredSteamIDs.Contains(id);
		}

		/// <summary>
		/// Close connection, and refuse all future connection attempts from a remote player.
		/// Used when garbage messages are received from hacked clients to avoid wasting time on them.
		/// </summary>
		// Token: 0x060036AB RID: 13995 RVA: 0x0010E234 File Offset: 0x0010C434
		public static void refuseGarbageConnection(CSteamID remoteId, string reason)
		{
			string[] array = new string[5];
			array[0] = "Refusing connections from ";
			int num = 1;
			CSteamID csteamID = remoteId;
			array[num] = csteamID.ToString();
			array[2] = " (";
			array[3] = reason;
			array[4] = ")";
			UnturnedLog.info(string.Concat(array));
			Provider.netIgnoredSteamIDs.Add(remoteId);
		}

		// Token: 0x060036AC RID: 13996 RVA: 0x0010E28C File Offset: 0x0010C48C
		public static void refuseGarbageConnection(ITransportConnection transportConnection, string reason)
		{
			if (transportConnection == null)
			{
				throw new ArgumentNullException("transportConnection");
			}
			transportConnection.CloseConnection();
			CSteamID csteamID = Provider.findTransportConnectionSteamId(transportConnection);
			if (csteamID != CSteamID.Nil)
			{
				Provider.refuseGarbageConnection(csteamID, reason);
			}
		}

		/// <summary>
		/// Should buffers used by plugin network events be read-only copies?
		/// </summary>
		// Token: 0x17000999 RID: 2457
		// (get) Token: 0x060036AD RID: 13997 RVA: 0x0010E2C8 File Offset: 0x0010C4C8
		public static bool useConstNetEvents
		{
			get
			{
				return Provider._constNetEvents;
			}
		}

		// Token: 0x060036AE RID: 13998 RVA: 0x0010E2D4 File Offset: 0x0010C4D4
		public static bool hasNetBufferChanged(byte[] original, byte[] copy, int offset, int size)
		{
			for (int i = offset + size - 1; i >= offset; i--)
			{
				if (copy[i] != original[i])
				{
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// First four bytes of RPC messages are the channel id.
		/// </summary>
		// Token: 0x060036AF RID: 13999 RVA: 0x0010E2FC File Offset: 0x0010C4FC
		internal static bool getChannelHeader(byte[] packet, int size, int offset, out int channel)
		{
			int num = offset + 2;
			if (num + 1 > size)
			{
				channel = -1;
				return false;
			}
			channel = (int)packet[num];
			return true;
		}

		/// <summary>
		/// Is version number supplied by client compatible with us?
		/// </summary>
		// Token: 0x060036B0 RID: 14000 RVA: 0x0010E31E File Offset: 0x0010C51E
		internal static bool canClientVersionJoinServer(uint version)
		{
			return version == Provider.APP_VERSION_PACKED;
		}

		// Token: 0x060036B1 RID: 14001 RVA: 0x0010E328 File Offset: 0x0010C528
		internal static void legacyReceiveClient(byte[] packet, int offset, int size)
		{
			CSteamID server = Provider.server;
			Provider._bytesReceived += (uint)size;
			Provider._packetsReceived += 1U;
			int id;
			if (!Provider.getChannelHeader(packet, size, offset, out id))
			{
				return;
			}
			SteamChannel steamChannel = Provider.findChannelComponent(id);
			if (steamChannel != null)
			{
				steamChannel.receive(server, packet, offset, size);
			}
		}

		// Token: 0x060036B2 RID: 14002 RVA: 0x0010E37C File Offset: 0x0010C57C
		private static void listenServer()
		{
			long num;
			ITransportConnection transportConnection;
			while (Provider.serverTransport.Receive(Provider.buffer, out num, out transportConnection))
			{
				NetMessages.ReceiveMessageFromClient(transportConnection, Provider.buffer, 0, (int)num);
			}
		}

		// Token: 0x060036B3 RID: 14003 RVA: 0x0010E3B0 File Offset: 0x0010C5B0
		private static void listenClient()
		{
			long num;
			while (Provider.clientTransport.Receive(Provider.buffer, out num))
			{
				NetMessages.ReceiveMessageFromServer(Provider.buffer, 0, (int)num);
			}
		}

		// Token: 0x060036B4 RID: 14004 RVA: 0x0010E3E0 File Offset: 0x0010C5E0
		private void SendPingRequestToAllClients()
		{
			float realtimeSinceStartup = Time.realtimeSinceStartup;
			foreach (SteamPlayer steamPlayer in Provider.clients)
			{
				if (realtimeSinceStartup - steamPlayer.timeLastPingRequestWasSentToClient > 1f || steamPlayer.timeLastPingRequestWasSentToClient < 0f)
				{
					steamPlayer.timeLastPingRequestWasSentToClient = realtimeSinceStartup;
					NetMessages.SendMessageToClient(EClientMessage.PingRequest, ENetReliability.Unreliable, steamPlayer.transportConnection, delegate(NetPakWriter writer)
					{
					});
				}
			}
		}

		/// <summary>
		/// Notify players waiting to join server if their position in the queue has changed.
		/// </summary>
		// Token: 0x060036B5 RID: 14005 RVA: 0x0010E480 File Offset: 0x0010C680
		private void NotifyClientsInQueueOfPosition()
		{
			int queuePosition;
			int queuePosition2;
			for (queuePosition = 0; queuePosition < Provider.pending.Count; queuePosition = queuePosition2)
			{
				if (Provider.pending[queuePosition].lastNotifiedQueuePosition != queuePosition)
				{
					Provider.pending[queuePosition].lastNotifiedQueuePosition = queuePosition;
					NetMessages.SendMessageToClient(EClientMessage.QueuePositionChanged, ENetReliability.Reliable, Provider.pending[queuePosition].transportConnection, delegate(NetPakWriter writer)
					{
						writer.WriteUInt8(MathfEx.ClampToByte(queuePosition));
					});
				}
				queuePosition2 = queuePosition + 1;
			}
		}

		// Token: 0x060036B6 RID: 14006 RVA: 0x0010E524 File Offset: 0x0010C724
		private void KickClientsWithBadConnection()
		{
			this.clientsWithBadConnecion.Clear();
			float realtimeSinceStartup = Time.realtimeSinceStartup;
			float num = 0f;
			int num2 = 0;
			int num3 = 0;
			int num4 = 0;
			foreach (SteamPlayer steamPlayer in Provider.clients)
			{
				float num5 = realtimeSinceStartup - steamPlayer.timeLastPacketWasReceivedFromClient;
				if (num5 > Provider.configData.Server.Timeout_Game_Seconds)
				{
					if (CommandWindow.shouldLogJoinLeave)
					{
						SteamPlayerID playerID = steamPlayer.playerID;
						CommandWindow.Log(Provider.localization.format("Dismiss_Timeout", playerID.steamID, playerID.playerName, playerID.characterName));
					}
					UnturnedLog.info(string.Format("Kicking {0} after {1} s without message", steamPlayer.transportConnection, num5));
					this.clientsWithBadConnecion.Add(steamPlayer);
					num += num5;
					num2++;
				}
				else if (realtimeSinceStartup - steamPlayer.joined > Provider.configData.Server.Timeout_Game_Seconds)
				{
					int num6 = Mathf.FloorToInt(steamPlayer.ping * 1000f);
					if ((long)num6 > (long)((ulong)Provider.configData.Server.Max_Ping_Milliseconds))
					{
						if (CommandWindow.shouldLogJoinLeave)
						{
							SteamPlayerID playerID2 = steamPlayer.playerID;
							CommandWindow.Log(Provider.localization.format("Dismiss_Ping", new object[]
							{
								num6,
								Provider.configData.Server.Max_Ping_Milliseconds,
								playerID2.steamID,
								playerID2.playerName,
								playerID2.characterName
							}));
						}
						UnturnedLog.info(string.Format("Kicking {0} because their ping ({1} ms) exceeds limit ({2} ms)", steamPlayer.transportConnection, num6, Provider.configData.Server.Max_Ping_Milliseconds));
						this.clientsWithBadConnecion.Add(steamPlayer);
						num3 += num6;
						num4++;
					}
				}
			}
			if (this.clientsWithBadConnecion.Count > 1)
			{
				UnturnedLog.info(string.Format("Kicking {0} clients with bad connection this frame. Maybe something blocked the main thread on the server? ({1} clients kicked of {2} total clients)", this.clientsWithBadConnecion.Count, this.clientsWithBadConnecion.Count, Provider.clients.Count));
				float num7 = (num2 > 0) ? (num / (float)this.clientsWithBadConnecion.Count) : 0f;
				UnturnedLog.info(string.Format("Kicking {0} for exceeding timeout limit ({1} s) with average of {2} s without message", num2, Provider.configData.Server.Timeout_Game_Seconds, num7));
				int num8 = (num4 > 0) ? (num3 / num4) : 0;
				UnturnedLog.info(string.Format("Kicking {0} for exceeding ping limit ({1} ms) with average of {2} ms ping", num4, Provider.configData.Server.Max_Ping_Milliseconds, num8));
			}
			foreach (SteamPlayer steamPlayer2 in this.clientsWithBadConnecion)
			{
				try
				{
					Provider.dismiss(steamPlayer2.playerID.steamID);
				}
				catch (Exception e)
				{
					UnturnedLog.exception(e, "Caught exception while kicking client for bad connection:");
				}
			}
		}

		/// <summary>
		/// Prevent any particular client from delaying the server connection queue process.
		/// </summary>
		// Token: 0x060036B7 RID: 14007 RVA: 0x0010E890 File Offset: 0x0010CA90
		private void KickClientsBlockingUpQueue()
		{
			if (Provider.pending.Count < 1)
			{
				return;
			}
			float clampedTimeoutQueueSeconds = Provider.configData.Server.GetClampedTimeoutQueueSeconds();
			SteamPending steamPending = Provider.pending[0];
			if (steamPending.hasSentVerifyPacket && steamPending.realtimeSinceSentVerifyPacket > clampedTimeoutQueueSeconds)
			{
				UnturnedLog.info("Front of queue player timed out: {0} ({1})", new object[]
				{
					steamPending.playerID.steamID,
					steamPending.GetQueueStateDebugString()
				});
				ESteamRejection rejection;
				if (!steamPending.hasAuthentication && steamPending.hasProof && steamPending.hasGroup)
				{
					rejection = ESteamRejection.LATE_PENDING_STEAM_AUTH;
					UnturnedLog.info(string.Format("Server was only waiting for Steam authentication response for front of queue player, but {0}s passed so we will give the next player a chance instead.", steamPending.realtimeSinceSentVerifyPacket));
				}
				else if (steamPending.hasAuthentication && !steamPending.hasProof && steamPending.hasGroup)
				{
					rejection = ESteamRejection.LATE_PENDING_STEAM_ECON;
					UnturnedLog.info(string.Format("Server was only waiting for Steam economy/inventory details response for front of queue player, but {0}s passed so we will give the next player a chance instead.", steamPending.realtimeSinceSentVerifyPacket));
				}
				else if (steamPending.hasAuthentication && steamPending.hasProof && !steamPending.hasGroup)
				{
					rejection = ESteamRejection.LATE_PENDING_STEAM_GROUPS;
					UnturnedLog.info(string.Format("Server was only waiting for Steam group/clan details response for front of queue player, but {0}s passed so we will give the next player a chance instead.", steamPending.realtimeSinceSentVerifyPacket));
				}
				else
				{
					rejection = ESteamRejection.LATE_PENDING;
					UnturnedLog.info(string.Format("Server was waiting for multiple responses about front of queue player, but {0}s passed so we will give the next player a chance instead.", steamPending.realtimeSinceSentVerifyPacket));
				}
				Provider.reject(steamPending.playerID.steamID, rejection);
				return;
			}
			if (Provider.pending.Count > 1)
			{
				float realtimeSinceStartup = Time.realtimeSinceStartup;
				for (int i = Provider.pending.Count - 1; i > 0; i--)
				{
					float num = realtimeSinceStartup - Provider.pending[i].lastReceivedPingRequestRealtime;
					if (num > Provider.configData.Server.Timeout_Queue_Seconds)
					{
						SteamPending steamPending2 = Provider.pending[i];
						UnturnedLog.info(string.Format("Kicking queued player {0} after {1}s without message", steamPending2.transportConnection, num));
						Provider.reject(steamPending2.playerID.steamID, ESteamRejection.LATE_PENDING);
						return;
					}
				}
			}
		}

		// Token: 0x060036B8 RID: 14008 RVA: 0x0010EA7C File Offset: 0x0010CC7C
		private static void listen()
		{
			if (!Provider.isConnected)
			{
				return;
			}
			if (Provider.isServer)
			{
				if (!Dedicator.IsDedicatedServer)
				{
					return;
				}
				if (!Level.isLoaded)
				{
					return;
				}
				TransportConnectionListPool.ReleaseAll();
				Provider.listenServer();
				if (Dedicator.IsDedicatedServer)
				{
					if (Time.realtimeSinceStartup - Provider.lastPingRequestTime > Provider.PING_REQUEST_INTERVAL)
					{
						Provider.lastPingRequestTime = Time.realtimeSinceStartup;
						Provider.steam.SendPingRequestToAllClients();
					}
					if (Time.realtimeSinceStartup - Provider.lastQueueNotificationTime > 6f)
					{
						Provider.lastQueueNotificationTime = Time.realtimeSinceStartup;
						Provider.steam.NotifyClientsInQueueOfPosition();
					}
					Provider.steam.KickClientsWithBadConnection();
					Provider.steam.KickClientsBlockingUpQueue();
					if (Provider.steam.clientsKickedForTransportConnectionFailureCount > 1)
					{
						UnturnedLog.info(string.Format("Removed {0} clients due to transport failure this frame", Provider.steam.clientsKickedForTransportConnectionFailureCount));
					}
					Provider.steam.clientsKickedForTransportConnectionFailureCount = 0;
				}
				if (Provider.dswUpdateMonitor != null)
				{
					Provider.dswUpdateMonitor.tick(Time.deltaTime);
					return;
				}
			}
			else
			{
				Provider.listenClient();
				if (!Provider.isConnected)
				{
					return;
				}
				if (Time.realtimeSinceStartup - Provider.lastPingRequestTime > Provider.PING_REQUEST_INTERVAL && (Time.realtimeSinceStartup - Provider.timeLastPingRequestWasSentToServer > 1f || Provider.timeLastPingRequestWasSentToServer < 0f))
				{
					Provider.lastPingRequestTime = Time.realtimeSinceStartup;
					Provider.timeLastPingRequestWasSentToServer = Time.realtimeSinceStartup;
					NetMessages.SendMessageToServer(EServerMessage.PingRequest, ENetReliability.Unreliable, delegate(NetPakWriter writer)
					{
					});
				}
				if (Provider.isLoadingUGC)
				{
					if (Provider.isWaitingForWorkshopResponse)
					{
						float num = Time.realtimeSinceStartup - Provider.timeLastPacketWasReceivedFromServer;
						if (num > (float)Provider.CLIENT_TIMEOUT)
						{
							Provider._connectionFailureInfo = ESteamConnectionFailureInfo.TIMED_OUT;
							Provider.RequestDisconnect(string.Format("Server did not reply to workshop details request ({0}s elapsed)", num));
						}
						return;
					}
					Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
					return;
				}
				else if (Level.isLoading)
				{
					float num2 = Time.realtimeSinceStartup - Provider.timeLastPacketWasReceivedFromServer;
					if (Provider.isWaitingForConnectResponse && num2 > 10f)
					{
						Provider._connectionFailureInfo = ESteamConnectionFailureInfo.TIMED_OUT;
						Provider.RequestDisconnect(string.Format("Server did not reply to connection request ({0}s elapsed)", num2));
						return;
					}
					if (Provider.isWaitingForAuthenticationResponse)
					{
						double num3 = Time.realtimeSinceStartupAsDouble - Provider.sentAuthenticationRequestTime;
						if (num3 > 30.0)
						{
							Provider._connectionFailureInfo = ESteamConnectionFailureInfo.TIMED_OUT_LOGIN;
							Provider.RequestDisconnect(string.Format("Server did not reply to authentication request ({0}s elapsed)", num3));
							return;
						}
					}
					Provider.timeLastPacketWasReceivedFromServer = Time.realtimeSinceStartup;
					return;
				}
				else
				{
					float num4 = Time.realtimeSinceStartup - Provider.timeLastPacketWasReceivedFromServer;
					if (num4 > (float)Provider.CLIENT_TIMEOUT)
					{
						Provider._connectionFailureInfo = ESteamConnectionFailureInfo.TIMED_OUT;
						Provider.RequestDisconnect(string.Format("it has been {0}s without a message from the server", num4));
						return;
					}
					if (Provider.battlEyeHasRequiredRestart)
					{
						Provider.battlEyeHasRequiredRestart = false;
						Provider.RequestDisconnect("BattlEye required restart");
						return;
					}
					ClientAssetIntegrity.SendRequests();
				}
			}
		}

		// Token: 0x060036B9 RID: 14009 RVA: 0x0010ED08 File Offset: 0x0010CF08
		private static void broadcastServerDisconnected(CSteamID steamID)
		{
			try
			{
				Provider.ServerDisconnected serverDisconnected = Provider.onServerDisconnected;
				if (serverDisconnected != null)
				{
					serverDisconnected(steamID);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onServerDisconnected:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036BA RID: 14010 RVA: 0x0010ED4C File Offset: 0x0010CF4C
		private static void broadcastServerHosted()
		{
			try
			{
				Provider.ServerHosted serverHosted = Provider.onServerHosted;
				if (serverHosted != null)
				{
					serverHosted();
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onServerHosted:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036BB RID: 14011 RVA: 0x0010ED8C File Offset: 0x0010CF8C
		private static void broadcastServerShutdown()
		{
			try
			{
				Provider.ServerShutdown serverShutdown = Provider.onServerShutdown;
				if (serverShutdown != null)
				{
					serverShutdown();
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onServerShutdown:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036BC RID: 14012 RVA: 0x0010EDCC File Offset: 0x0010CFCC
		private static void onP2PSessionConnectFail(P2PSessionConnectFail_t callback)
		{
			UnturnedLog.info(string.Format("Removing player {0} due to P2P connect failure (Error: {1})", callback.m_steamIDRemote, callback.m_eP2PSessionError));
			Provider.dismiss(callback.m_steamIDRemote);
		}

		// Token: 0x060036BD RID: 14013 RVA: 0x0010EE00 File Offset: 0x0010D000
		internal static void checkBanStatus(SteamPlayerID playerID, uint remoteIP, out bool isBanned, out string banReason, out uint banRemainingDuration)
		{
			isBanned = false;
			banReason = string.Empty;
			banRemainingDuration = 0U;
			SteamBlacklistID steamBlacklistID;
			if (SteamBlacklist.checkBanned(playerID.steamID, remoteIP, playerID.GetHwids(), out steamBlacklistID))
			{
				isBanned = true;
				banReason = steamBlacklistID.reason;
				banRemainingDuration = steamBlacklistID.getTime();
			}
			try
			{
				if (Provider.onCheckBanStatusWithHWID != null)
				{
					Provider.onCheckBanStatusWithHWID(playerID, remoteIP, ref isBanned, ref banReason, ref banRemainingDuration);
				}
				else if (Provider.onCheckBanStatus != null)
				{
					Provider.onCheckBanStatus(playerID.steamID, remoteIP, ref isBanned, ref banReason, ref banRemainingDuration);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onCheckBanStatus:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036BE RID: 14014 RVA: 0x0010EEA0 File Offset: 0x0010D0A0
		[Obsolete("Now accepts list of HWIDs to ban")]
		public static bool requestBanPlayer(CSteamID instigator, CSteamID playerToBan, uint ipToBan, string reason, uint duration)
		{
			return Provider.requestBanPlayer(instigator, playerToBan, ipToBan, null, reason, duration);
		}

		// Token: 0x060036BF RID: 14015 RVA: 0x0010EEB0 File Offset: 0x0010D0B0
		public static bool requestBanPlayer(CSteamID instigator, CSteamID playerToBan, uint ipToBan, IEnumerable<byte[]> hwidsToBan, string reason, uint duration)
		{
			bool flag = true;
			try
			{
				Provider.RequestBanPlayerHandler requestBanPlayerHandler = Provider.onBanPlayerRequested;
				if (requestBanPlayerHandler != null)
				{
					requestBanPlayerHandler(instigator, playerToBan, ipToBan, ref reason, ref duration, ref flag);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.exception(e, "Plugin raised an exception from onBanPlayerRequested:");
			}
			try
			{
				Provider.RequestBanPlayerHandlerV2 requestBanPlayerHandlerV = Provider.onBanPlayerRequestedV2;
				if (requestBanPlayerHandlerV != null)
				{
					requestBanPlayerHandlerV(instigator, playerToBan, ipToBan, hwidsToBan, ref reason, ref duration, ref flag);
				}
			}
			catch (Exception e2)
			{
				UnturnedLog.exception(e2, "Plugin raised an exception from onBanPlayerRequestedV2:");
			}
			if (flag)
			{
				SteamBlacklist.ban(playerToBan, ipToBan, hwidsToBan, instigator, reason, duration);
			}
			return true;
		}

		// Token: 0x060036C0 RID: 14016 RVA: 0x0010EF3C File Offset: 0x0010D13C
		public static bool requestUnbanPlayer(CSteamID instigator, CSteamID playerToUnban)
		{
			bool flag = true;
			try
			{
				Provider.RequestUnbanPlayerHandler requestUnbanPlayerHandler = Provider.onUnbanPlayerRequested;
				if (requestUnbanPlayerHandler != null)
				{
					requestUnbanPlayerHandler(instigator, playerToUnban, ref flag);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onUnbanPlayerRequested:");
				UnturnedLog.exception(e);
			}
			return !flag || SteamBlacklist.unban(playerToUnban);
		}

		// Token: 0x060036C1 RID: 14017 RVA: 0x0010EF90 File Offset: 0x0010D190
		private static void handleValidateAuthTicketResponse(ValidateAuthTicketResponse_t callback)
		{
			if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseOK)
			{
				SteamPending steamPending = null;
				for (int i = 0; i < Provider.pending.Count; i++)
				{
					if (Provider.pending[i].playerID.steamID == callback.m_SteamID)
					{
						steamPending = Provider.pending[i];
						break;
					}
				}
				if (steamPending == null)
				{
					for (int j = 0; j < Provider.clients.Count; j++)
					{
						if (Provider.clients[j].playerID.steamID == callback.m_SteamID)
						{
							return;
						}
					}
					Provider.reject(callback.m_SteamID, ESteamRejection.NOT_PENDING);
					return;
				}
				bool flag = true;
				string empty = string.Empty;
				try
				{
					if (Provider.onCheckValidWithExplanation != null)
					{
						Provider.onCheckValidWithExplanation(callback, ref flag, ref empty);
					}
					else if (Provider.onCheckValid != null)
					{
						Provider.onCheckValid(callback, ref flag);
					}
				}
				catch (Exception e)
				{
					UnturnedLog.warn("Plugin raised an exception from onCheckValidWithExplanation or onCheckValid:");
					UnturnedLog.exception(e);
				}
				if (!flag)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.PLUGIN, empty);
					return;
				}
				bool flag2 = SteamGameServer.UserHasLicenseForApp(steamPending.playerID.steamID, Provider.PRO_ID) != EUserHasLicenseForAppResult.k_EUserHasLicenseResultDoesNotHaveLicense;
				if (Provider.isGold && !flag2)
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_SERVER);
					return;
				}
				if ((steamPending.playerID.characterID >= Customization.FREE_CHARACTERS && !flag2) || steamPending.playerID.characterID >= Customization.FREE_CHARACTERS + Customization.PRO_CHARACTERS)
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_CHARACTER);
					return;
				}
				if (!flag2 && steamPending.isPro)
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_DESYNC);
					return;
				}
				if (steamPending.face >= Customization.FACES_FREE + Customization.FACES_PRO || (!flag2 && steamPending.face >= Customization.FACES_FREE))
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_APPEARANCE);
					return;
				}
				if (steamPending.hair >= Customization.HAIRS_FREE + Customization.HAIRS_PRO || (!flag2 && steamPending.hair >= Customization.HAIRS_FREE))
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_APPEARANCE);
					return;
				}
				if (steamPending.beard >= Customization.BEARDS_FREE + Customization.BEARDS_PRO || (!flag2 && steamPending.beard >= Customization.BEARDS_FREE))
				{
					Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_APPEARANCE);
					return;
				}
				if (!flag2)
				{
					if (!Customization.checkSkin(steamPending.skin))
					{
						Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_APPEARANCE);
						return;
					}
					if (!Customization.checkColor(steamPending.color))
					{
						Provider.reject(steamPending.playerID.steamID, ESteamRejection.PRO_APPEARANCE);
						return;
					}
				}
				steamPending.assignedPro = flag2;
				steamPending.assignedAdmin = SteamAdminlist.checkAdmin(steamPending.playerID.steamID);
				steamPending.hasAuthentication = true;
				if (steamPending.canAcceptYet)
				{
					Provider.accept(steamPending);
					return;
				}
			}
			else
			{
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseUserNotConnectedToSteam)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_NO_STEAM);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseNoLicenseOrExpired)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_LICENSE_EXPIRED);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseVACBanned)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_VAC_BAN);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseLoggedInElseWhere)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_ELSEWHERE);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseVACCheckTimedOut)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_TIMED_OUT);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseAuthTicketCanceled)
				{
					if (CommandWindow.shouldLogJoinLeave)
					{
						string str = "Player finished session: ";
						CSteamID steamID = callback.m_SteamID;
						CommandWindow.Log(str + steamID.ToString());
					}
					else
					{
						string str2 = "Player finished session: ";
						CSteamID steamID = callback.m_SteamID;
						UnturnedLog.info(str2 + steamID.ToString());
					}
					Provider.dismiss(callback.m_SteamID);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseAuthTicketInvalidAlreadyUsed)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_USED);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseAuthTicketInvalid)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_NO_USER);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponsePublisherIssuedBan)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_PUB_BAN);
					return;
				}
				if (callback.m_eAuthSessionResponse == EAuthSessionResponse.k_EAuthSessionResponseAuthTicketNetworkIdentityFailure)
				{
					Provider.reject(callback.m_SteamID, ESteamRejection.AUTH_NETWORK_IDENTITY_FAILURE);
					return;
				}
				if (CommandWindow.shouldLogJoinLeave)
				{
					string str3 = "Kicking player ";
					CSteamID steamID = callback.m_SteamID;
					CommandWindow.Log(str3 + steamID.ToString() + " for unknown session response " + callback.m_eAuthSessionResponse.ToString());
				}
				else
				{
					string str4 = "Kicking player ";
					CSteamID steamID = callback.m_SteamID;
					UnturnedLog.info(str4 + steamID.ToString() + " for unknown session response " + callback.m_eAuthSessionResponse.ToString());
				}
				Provider.dismiss(callback.m_SteamID);
			}
		}

		// Token: 0x060036C2 RID: 14018 RVA: 0x0010F420 File Offset: 0x0010D620
		private static void onValidateAuthTicketResponse(ValidateAuthTicketResponse_t callback)
		{
			Provider.handleValidateAuthTicketResponse(callback);
		}

		// Token: 0x060036C3 RID: 14019 RVA: 0x0010F428 File Offset: 0x0010D628
		private static void handleClientGroupStatus(GSClientGroupStatus_t callback)
		{
			SteamPending steamPending = null;
			for (int i = 0; i < Provider.pending.Count; i++)
			{
				if (Provider.pending[i].playerID.steamID == callback.m_SteamIDUser)
				{
					steamPending = Provider.pending[i];
					break;
				}
			}
			if (steamPending == null)
			{
				Provider.reject(callback.m_SteamIDUser, ESteamRejection.NOT_PENDING);
				return;
			}
			if (!callback.m_bMember && !callback.m_bOfficer)
			{
				steamPending.playerID.group = CSteamID.Nil;
			}
			steamPending.hasGroup = true;
			if (steamPending.canAcceptYet)
			{
				Provider.accept(steamPending);
			}
		}

		// Token: 0x060036C4 RID: 14020 RVA: 0x0010F4C2 File Offset: 0x0010D6C2
		private static void onClientGroupStatus(GSClientGroupStatus_t callback)
		{
			Provider.handleClientGroupStatus(callback);
		}

		// Token: 0x1700099A RID: 2458
		// (get) Token: 0x060036C5 RID: 14021 RVA: 0x0010F4CA File Offset: 0x0010D6CA
		// (set) Token: 0x060036C6 RID: 14022 RVA: 0x0010F4D4 File Offset: 0x0010D6D4
		public static byte maxPlayers
		{
			get
			{
				return Provider._maxPlayers;
			}
			set
			{
				Provider._maxPlayers = value;
				if (Provider.clMaxPlayersLimit.hasValue && (int)Provider.maxPlayers > Provider.clMaxPlayersLimit.value)
				{
					Provider._maxPlayers = (byte)Provider.clMaxPlayersLimit.value;
					UnturnedLog.info("Clamped max players down from {0} to {1}", new object[]
					{
						value,
						Provider.clMaxPlayersLimit.value
					});
				}
				if (Provider.isServer)
				{
					SteamGameServer.SetMaxPlayerCount((int)Provider.maxPlayers);
				}
			}
		}

		// Token: 0x1700099B RID: 2459
		// (get) Token: 0x060036C7 RID: 14023 RVA: 0x0010F550 File Offset: 0x0010D750
		public static byte queuePosition
		{
			get
			{
				return Provider._queuePosition;
			}
		}

		// Token: 0x1700099C RID: 2460
		// (get) Token: 0x060036C8 RID: 14024 RVA: 0x0010F557 File Offset: 0x0010D757
		// (set) Token: 0x060036C9 RID: 14025 RVA: 0x0010F55E File Offset: 0x0010D75E
		public static string serverName
		{
			get
			{
				return Provider._serverName;
			}
			set
			{
				Provider._serverName = value;
				if (Dedicator.commandWindow != null)
				{
					Dedicator.commandWindow.title = Provider.serverName;
				}
				if (Provider.isServer)
				{
					SteamGameServer.SetServerName(Provider.serverName);
				}
			}
		}

		// Token: 0x1700099D RID: 2461
		// (get) Token: 0x060036CA RID: 14026 RVA: 0x0010F58D File Offset: 0x0010D78D
		// (set) Token: 0x060036CB RID: 14027 RVA: 0x0010F594 File Offset: 0x0010D794
		public static string serverID
		{
			get
			{
				return Dedicator.serverID;
			}
			set
			{
				Dedicator.serverID = value;
			}
		}

		/// <summary>
		/// If hosting a server, get the game traffic port.
		/// </summary>
		// Token: 0x060036CC RID: 14028 RVA: 0x0010F59C File Offset: 0x0010D79C
		public static ushort GetServerConnectionPort()
		{
			return Provider.port + 1;
		}

		// Token: 0x1700099E RID: 2462
		// (get) Token: 0x060036CD RID: 14029 RVA: 0x0010F5A6 File Offset: 0x0010D7A6
		public static byte[] serverPasswordHash
		{
			get
			{
				return Provider._serverPasswordHash;
			}
		}

		// Token: 0x1700099F RID: 2463
		// (get) Token: 0x060036CE RID: 14030 RVA: 0x0010F5AD File Offset: 0x0010D7AD
		// (set) Token: 0x060036CF RID: 14031 RVA: 0x0010F5B4 File Offset: 0x0010D7B4
		public static string serverPassword
		{
			get
			{
				return Provider._serverPassword;
			}
			set
			{
				Provider._serverPassword = value;
				Provider._serverPasswordHash = Hash.SHA1(Provider.serverPassword);
				if (Provider.isServer)
				{
					SteamGameServer.SetPasswordProtected(Provider.serverPassword != "");
				}
			}
		}

		// Token: 0x170009A0 RID: 2464
		// (get) Token: 0x060036D0 RID: 14032 RVA: 0x0010F5E6 File Offset: 0x0010D7E6
		public static StatusData statusData
		{
			get
			{
				return Provider._statusData;
			}
		}

		// Token: 0x170009A1 RID: 2465
		// (get) Token: 0x060036D1 RID: 14033 RVA: 0x0010F5ED File Offset: 0x0010D7ED
		public static PreferenceData preferenceData
		{
			get
			{
				return Provider._preferenceData;
			}
		}

		// Token: 0x170009A2 RID: 2466
		// (get) Token: 0x060036D2 RID: 14034 RVA: 0x0010F5F4 File Offset: 0x0010D7F4
		public static ConfigData configData
		{
			get
			{
				return Provider._configData;
			}
		}

		// Token: 0x170009A3 RID: 2467
		// (get) Token: 0x060036D3 RID: 14035 RVA: 0x0010F5FB File Offset: 0x0010D7FB
		public static ModeConfigData modeConfigData
		{
			get
			{
				return Provider._modeConfigData;
			}
		}

		/// <summary>
		/// Called while running
		/// </summary>
		// Token: 0x060036D4 RID: 14036 RVA: 0x0010F604 File Offset: 0x0010D804
		public static void resetConfig()
		{
			Provider._modeConfigData = new ModeConfigData(Provider.mode);
			switch (Provider.mode)
			{
			case EGameMode.EASY:
				Provider.configData.Easy = Provider.modeConfigData;
				break;
			case EGameMode.NORMAL:
				Provider.configData.Normal = Provider.modeConfigData;
				break;
			case EGameMode.HARD:
				Provider.configData.Hard = Provider.modeConfigData;
				break;
			}
			ServerSavedata.serializeJSON<ConfigData>("/Config.json", Provider.configData);
		}

		// Token: 0x060036D5 RID: 14037 RVA: 0x0010F67C File Offset: 0x0010D87C
		private static void applyLevelConfigOverride(FieldInfo field, object targetObject, object defaultTargetObject, KeyValuePair<string, object> levelOverride)
		{
			object value = field.GetValue(targetObject);
			object value2 = field.GetValue(defaultTargetObject);
			Type fieldType = field.FieldType;
			bool flag3;
			if (fieldType == typeof(bool))
			{
				bool flag = (bool)value;
				bool flag2 = (bool)value2;
				flag3 = (flag == flag2);
				if (flag3)
				{
					field.SetValue(targetObject, Convert.ToBoolean(levelOverride.Value));
				}
			}
			else if (fieldType == typeof(float))
			{
				float a = (float)value;
				float b = (float)value2;
				flag3 = MathfEx.IsNearlyEqual(a, b, 0.0001f);
				if (flag3)
				{
					field.SetValue(targetObject, Convert.ToSingle(levelOverride.Value));
				}
			}
			else
			{
				if (!(fieldType == typeof(uint)))
				{
					CommandWindow.LogErrorFormat("Unable to handle level mode config override type: {0} ({1})", new object[]
					{
						fieldType,
						levelOverride.Key
					});
					return;
				}
				uint num = (uint)value;
				uint num2 = (uint)value2;
				flag3 = (num == num2);
				if (flag3)
				{
					field.SetValue(targetObject, Convert.ToUInt32(levelOverride.Value));
				}
			}
			if (!flag3)
			{
				CommandWindow.LogFormat("Skipping level config override {0} because server value ({1}) is not the default ({2})", new object[]
				{
					levelOverride.Key,
					value,
					value2
				});
				return;
			}
			CommandWindow.LogFormat("Level overrides config {0}: {1} (Default: {2})", new object[]
			{
				levelOverride.Key,
				levelOverride.Value,
				value2
			});
		}

		// Token: 0x060036D6 RID: 14038 RVA: 0x0010F7E4 File Offset: 0x0010D9E4
		public static void applyLevelModeConfigOverrides()
		{
			if (Level.info == null || Level.info.configData == null)
			{
				return;
			}
			ModeConfigData modeConfig = ConfigData.CreateDefault(!Dedicator.IsDedicatedServer).getModeConfig(Provider.mode);
			if (modeConfig == null)
			{
				CommandWindow.LogError("Unable to compare default for level mode config overrides");
				return;
			}
			foreach (KeyValuePair<string, object> levelOverride in Level.info.configData.Mode_Config_Overrides)
			{
				if (string.IsNullOrEmpty(levelOverride.Key))
				{
					CommandWindow.LogError("Level mode config overrides contains an empty key");
					break;
				}
				if (levelOverride.Value == null)
				{
					CommandWindow.LogError("Level mode config overrides contains a null value");
					break;
				}
				Type type = typeof(ModeConfigData);
				object obj = Provider.modeConfigData;
				object obj2 = modeConfig;
				string[] array = levelOverride.Key.Split('.', StringSplitOptions.None);
				int i = 0;
				while (i < array.Length)
				{
					string text = array[i];
					FieldInfo field = type.GetField(text);
					if (field == null)
					{
						CommandWindow.LogError("Failed to find mode config for level override: " + text);
						break;
					}
					if (i == array.Length - 1)
					{
						try
						{
							Provider.applyLevelConfigOverride(field, obj, obj2, levelOverride);
							goto IL_140;
						}
						catch (Exception e)
						{
							CommandWindow.LogError("Exception when applying level config override: " + levelOverride.Key);
							UnturnedLog.exception(e);
							break;
						}
						goto IL_122;
					}
					goto IL_122;
					IL_140:
					i++;
					continue;
					IL_122:
					type = field.FieldType;
					obj = field.GetValue(obj);
					obj2 = field.GetValue(obj2);
					goto IL_140;
				}
			}
		}

		// Token: 0x060036D7 RID: 14039 RVA: 0x0010F994 File Offset: 0x0010DB94
		public static void accept(SteamPending player)
		{
			Provider.accept(player.playerID, player.assignedPro, player.assignedAdmin, player.face, player.hair, player.beard, player.skin, player.color, player.markerColor, player.hand, player.shirtItem, player.pantsItem, player.hatItem, player.backpackItem, player.vestItem, player.maskItem, player.glassesItem, player.skinItems, player.skinTags, player.skinDynamicProps, player.skillset, player.language, player.lobbyID, player.clientPlatform);
		}

		/// <summary>
		/// Used to build packet about each existing player for new player, and then once to build a packet
		/// for existing players about the new player. Note that in this second case forPlayer is null
		/// because the packet is re-used.
		/// </summary>
		// Token: 0x060036D8 RID: 14040 RVA: 0x0010FA38 File Offset: 0x0010DC38
		private static void WriteConnectedMessage(NetPakWriter writer, SteamPlayer aboutPlayer, SteamPlayer forPlayer)
		{
			writer.WriteNetId(aboutPlayer.GetNetId());
			writer.WriteSteamID(aboutPlayer.playerID.steamID);
			writer.WriteUInt8(aboutPlayer.playerID.characterID);
			writer.WriteString(aboutPlayer.playerID.playerName, 11);
			writer.WriteString(aboutPlayer.playerID.characterName, 11);
			writer.WriteClampedVector3(aboutPlayer.model.transform.position, 13, 7);
			byte value = (byte)(aboutPlayer.model.transform.rotation.eulerAngles.y / 2f);
			writer.WriteUInt8(value);
			writer.WriteBit(aboutPlayer.isPro);
			bool value2 = aboutPlayer.isAdmin;
			if (forPlayer != aboutPlayer && Provider.hideAdmins)
			{
				value2 = false;
			}
			writer.WriteBit(value2);
			writer.WriteUInt8((byte)aboutPlayer.channel);
			writer.WriteSteamID(aboutPlayer.playerID.group);
			writer.WriteString(aboutPlayer.playerID.nickName, 11);
			writer.WriteUInt8(aboutPlayer.face);
			writer.WriteUInt8(aboutPlayer.hair);
			writer.WriteUInt8(aboutPlayer.beard);
			writer.WriteColor32RGB(aboutPlayer.skin);
			writer.WriteColor32RGB(aboutPlayer.color);
			writer.WriteColor32RGB(aboutPlayer.markerColor);
			writer.WriteBit(aboutPlayer.IsLeftHanded);
			writer.WriteInt32(aboutPlayer.shirtItem);
			writer.WriteInt32(aboutPlayer.pantsItem);
			writer.WriteInt32(aboutPlayer.hatItem);
			writer.WriteInt32(aboutPlayer.backpackItem);
			writer.WriteInt32(aboutPlayer.vestItem);
			writer.WriteInt32(aboutPlayer.maskItem);
			writer.WriteInt32(aboutPlayer.glassesItem);
			int[] skinItems = aboutPlayer.skinItems;
			writer.WriteUInt8((byte)skinItems.Length);
			foreach (int value3 in skinItems)
			{
				writer.WriteInt32(value3);
			}
			string[] skinTags = aboutPlayer.skinTags;
			writer.WriteUInt8((byte)skinTags.Length);
			foreach (string value4 in skinTags)
			{
				writer.WriteString(value4, 11);
			}
			string[] skinDynamicProps = aboutPlayer.skinDynamicProps;
			writer.WriteUInt8((byte)skinDynamicProps.Length);
			foreach (string value5 in skinDynamicProps)
			{
				writer.WriteString(value5, 11);
			}
			writer.WriteEnum(aboutPlayer.skillset);
			writer.WriteString(aboutPlayer.language, 11);
		}

		/// <summary>
		/// Not exactly ideal, but this a few old "once per player" client-&gt;server RPCs.
		/// </summary>
		// Token: 0x060036D9 RID: 14041 RVA: 0x0010FCD3 File Offset: 0x0010DED3
		private static void SendInitialGlobalState(SteamPlayer client)
		{
			LightingManager.SendInitialGlobalState(client);
			VehicleManager.SendInitialGlobalState(client);
			AnimalManager.SendInitialGlobalState(client.transportConnection);
			LevelManager.SendInitialGlobalState(client);
			ZombieManager.SendInitialGlobalState(client);
		}

		// Token: 0x060036DA RID: 14042 RVA: 0x0010FCF8 File Offset: 0x0010DEF8
		[Obsolete("This should not have been public in the first place")]
		public static void accept(SteamPlayerID playerID, bool isPro, bool isAdmin, byte face, byte hair, byte beard, Color skin, Color color, Color markerColor, bool hand, int shirtItem, int pantsItem, int hatItem, int backpackItem, int vestItem, int maskItem, int glassesItem, int[] skinItems, string[] skinTags, string[] skinDynamicProps, EPlayerSkillset skillset, string language, CSteamID lobbyID)
		{
			Provider.accept(playerID, isPro, isAdmin, face, hair, beard, skin, color, markerColor, hand, shirtItem, pantsItem, hatItem, backpackItem, vestItem, maskItem, glassesItem, skinItems, skinTags, skinDynamicProps, skillset, language, lobbyID, EClientPlatform.Windows);
		}

		// Token: 0x060036DB RID: 14043 RVA: 0x0010FD38 File Offset: 0x0010DF38
		internal static void accept(SteamPlayerID playerID, bool isPro, bool isAdmin, byte face, byte hair, byte beard, Color skin, Color color, Color markerColor, bool hand, int shirtItem, int pantsItem, int hatItem, int backpackItem, int vestItem, int maskItem, int glassesItem, int[] skinItems, string[] skinTags, string[] skinDynamicProps, EPlayerSkillset skillset, string language, CSteamID lobbyID, EClientPlatform clientPlatform)
		{
			ITransportConnection transportConnection = null;
			bool flag = false;
			int num = 0;
			for (int i = 0; i < Provider.pending.Count; i++)
			{
				if (Provider.pending[i].playerID == playerID)
				{
					if (Provider.pending[i].inventoryResult != SteamInventoryResult_t.Invalid)
					{
						SteamGameServerInventory.DestroyResult(Provider.pending[i].inventoryResult);
						Provider.pending[i].inventoryResult = SteamInventoryResult_t.Invalid;
					}
					transportConnection = Provider.pending[i].transportConnection;
					flag = true;
					num = i;
					Provider.pending.RemoveAt(i);
					break;
				}
			}
			if (!flag)
			{
				UnturnedLog.info(string.Format("Ignoring call to accept {0} because they are not in the queue", playerID));
				return;
			}
			UnturnedLog.info(string.Format("Accepting queued player {0}", playerID));
			string characterName = playerID.characterName;
			uint uScore = isPro ? 1U : 0U;
			SteamGameServer.BUpdateUserData(playerID.steamID, characterName, uScore);
			Vector3 point;
			byte angle;
			EPlayerStance initialStance;
			Provider.loadPlayerSpawn(playerID, out point, out angle, out initialStance);
			int channel = Provider.allocPlayerChannelId();
			NetId netId = Provider.ClaimNetIdBlockForNewPlayer();
			SteamPlayer newClient = Provider.addPlayer(transportConnection, netId, playerID, point, angle, isPro, isAdmin, channel, face, hair, beard, skin, color, markerColor, hand, shirtItem, pantsItem, hatItem, backpackItem, vestItem, maskItem, glassesItem, skinItems, skinTags, skinDynamicProps, skillset, language, lobbyID, clientPlatform);
			newClient.battlEyeId = Provider.allocBattlEyePlayerId();
			PlayerStance component = newClient.player.GetComponent<PlayerStance>();
			if (component != null)
			{
				component.initialStance = initialStance;
			}
			else
			{
				UnturnedLog.warn("Was unable to get PlayerStance for new connection!");
			}
			using (List<SteamPlayer>.Enumerator enumerator = Provider._clients.GetEnumerator())
			{
				while (enumerator.MoveNext())
				{
					SteamPlayer aboutClient = enumerator.Current;
					NetMessages.SendMessageToClient(EClientMessage.PlayerConnected, ENetReliability.Reliable, newClient.transportConnection, delegate(NetPakWriter writer)
					{
						Provider.WriteConnectedMessage(writer, aboutClient, newClient);
					});
				}
			}
			uint ipForClient;
			SteamGameServer.GetPublicIP().TryGetIPv4Address(out ipForClient);
			NetMessages.SendMessageToClient(EClientMessage.Accepted, ENetReliability.Reliable, transportConnection, delegate(NetPakWriter writer)
			{
				writer.WriteUInt32(ipForClient);
				writer.WriteUInt16(Provider.port);
				writer.WriteUInt8((byte)Provider.modeConfigData.Gameplay.Repair_Level_Max);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Hitmarkers);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Crosshair);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Ballistics);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Chart);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Satellite);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Compass);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Group_Map);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Group_HUD);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Group_Player_List);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Allow_Static_Groups);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Allow_Dynamic_Groups);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Allow_Shoulder_Camera);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Can_Suicide);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Friendly_Fire);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Bypass_Buildable_Mobility);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Allow_Freeform_Buildables);
				writer.WriteBit(Provider.modeConfigData.Gameplay.Allow_Freeform_Buildables_On_Vehicles);
				writer.WriteUInt16((ushort)Provider.modeConfigData.Gameplay.Timer_Exit);
				writer.WriteUInt16((ushort)Provider.modeConfigData.Gameplay.Timer_Respawn);
				writer.WriteUInt16((ushort)Provider.modeConfigData.Gameplay.Timer_Home);
				writer.WriteUInt16((ushort)Provider.modeConfigData.Gameplay.Max_Group_Members);
				writer.WriteBit(Provider.modeConfigData.Barricades.Allow_Item_Placement_On_Vehicle);
				writer.WriteBit(Provider.modeConfigData.Barricades.Allow_Trap_Placement_On_Vehicle);
				writer.WriteFloat(Provider.modeConfigData.Barricades.Max_Item_Distance_From_Hull);
				writer.WriteFloat(Provider.modeConfigData.Barricades.Max_Trap_Distance_From_Hull);
				writer.WriteFloat(Provider.modeConfigData.Gameplay.AirStrafing_Acceleration_Multiplier);
				writer.WriteFloat(Provider.modeConfigData.Gameplay.AirStrafing_Deceleration_Multiplier);
				writer.WriteFloat(Provider.modeConfigData.Gameplay.ThirdPerson_RecoilMultiplier);
				writer.WriteFloat(Provider.modeConfigData.Gameplay.ThirdPerson_SpreadMultiplier);
			});
			if (Provider.battlEyeServerHandle != IntPtr.Zero && Provider.battlEyeServerRunData != null && Provider.battlEyeServerRunData.pfnAddPlayer != null && Provider.battlEyeServerRunData.pfnReceivedPlayerGUID != null)
			{
				uint ipv4AddressOrZero = newClient.getIPv4AddressOrZero();
				ushort num2;
				transportConnection.TryGetPort(out num2);
				uint ulAddress = (ipv4AddressOrZero & 255U) << 24 | (ipv4AddressOrZero & 65280U) << 8 | (ipv4AddressOrZero & 16711680U) >> 8 | (ipv4AddressOrZero & 4278190080U) >> 24;
				ushort usPort = (ushort)((int)(num2 & 255) << 8 | (int)((uint)(num2 & 65280) >> 8));
				Provider.battlEyeServerRunData.pfnAddPlayer(newClient.battlEyeId, ulAddress, usPort, playerID.playerName);
				GCHandle gchandle = GCHandle.Alloc(playerID.steamID.m_SteamID, GCHandleType.Pinned);
				IntPtr pvGUID = gchandle.AddrOfPinnedObject();
				Provider.battlEyeServerRunData.pfnReceivedPlayerGUID(newClient.battlEyeId, pvGUID, 8);
				gchandle.Free();
			}
			NetMessages.SendMessageToClients(EClientMessage.PlayerConnected, ENetReliability.Reliable, Provider.GatherRemoteClientConnectionsMatchingPredicate((SteamPlayer potentialRecipient) => potentialRecipient != newClient), delegate(NetPakWriter writer)
			{
				Provider.WriteConnectedMessage(writer, newClient, null);
			});
			Provider.SendInitialGlobalState(newClient);
			newClient.player.InitializePlayer();
			foreach (SteamPlayer steamPlayer in Provider._clients)
			{
				steamPlayer.player.SendInitialPlayerState(newClient);
			}
			newClient.player.SendInitialPlayerState(Provider.GatherRemoteClientConnectionsMatchingPredicate((SteamPlayer potentialRecipient) => potentialRecipient != newClient));
			try
			{
				Provider.ServerConnected serverConnected = Provider.onServerConnected;
				if (serverConnected != null)
				{
					serverConnected(playerID.steamID);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onServerConnected:");
				UnturnedLog.exception(e);
			}
			if (CommandWindow.shouldLogJoinLeave)
			{
				CommandWindow.Log(Provider.localization.format("PlayerConnectedText", playerID.steamID, playerID.playerName, playerID.characterName));
			}
			else
			{
				UnturnedLog.info(Provider.localization.format("PlayerConnectedText", playerID.steamID, playerID.playerName, playerID.characterName));
			}
			if (num == 0)
			{
				Provider.verifyNextPlayerInQueue();
			}
		}

		/// <summary>
		/// Event for plugins when rejecting a player.
		/// </summary>
		// Token: 0x140000D4 RID: 212
		// (add) Token: 0x060036DC RID: 14044 RVA: 0x001101C4 File Offset: 0x0010E3C4
		// (remove) Token: 0x060036DD RID: 14045 RVA: 0x001101F8 File Offset: 0x0010E3F8
		public static event Provider.RejectingPlayerCallback onRejectingPlayer;

		// Token: 0x060036DE RID: 14046 RVA: 0x0011022C File Offset: 0x0010E42C
		private static void broadcastRejectingPlayer(CSteamID steamID, ESteamRejection rejection, string explanation)
		{
			try
			{
				Provider.RejectingPlayerCallback rejectingPlayerCallback = Provider.onRejectingPlayer;
				if (rejectingPlayerCallback != null)
				{
					rejectingPlayerCallback(steamID, rejection, explanation);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Plugin raised an exception from onRejectingPlayer:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036DF RID: 14047 RVA: 0x00110270 File Offset: 0x0010E470
		public static void reject(CSteamID steamID, ESteamRejection rejection)
		{
			Provider.reject(steamID, rejection, string.Empty);
		}

		// Token: 0x060036E0 RID: 14048 RVA: 0x00110280 File Offset: 0x0010E480
		public static void reject(CSteamID steamID, ESteamRejection rejection, string explanation)
		{
			ITransportConnection transportConnection = Provider.findTransportConnection(steamID);
			if (transportConnection != null)
			{
				Provider.reject(transportConnection, rejection, explanation);
			}
		}

		// Token: 0x060036E1 RID: 14049 RVA: 0x0011029F File Offset: 0x0010E49F
		public static void reject(ITransportConnection transportConnection, ESteamRejection rejection)
		{
			Provider.reject(transportConnection, rejection, string.Empty);
		}

		// Token: 0x060036E2 RID: 14050 RVA: 0x001102B0 File Offset: 0x0010E4B0
		public static void reject(ITransportConnection transportConnection, ESteamRejection rejection, string explanation)
		{
			if (transportConnection == null)
			{
				throw new ArgumentNullException("transportConnection");
			}
			CSteamID csteamID = Provider.findTransportConnectionSteamId(transportConnection);
			if (csteamID != CSteamID.Nil)
			{
				Provider.broadcastRejectingPlayer(csteamID, rejection, explanation);
			}
			int i = 0;
			while (i < Provider.pending.Count)
			{
				if (transportConnection.Equals(Provider.pending[i].transportConnection))
				{
					if (rejection == ESteamRejection.AUTH_VAC_BAN)
					{
						ChatManager.say(Provider.pending[i].playerID.playerName + " was banned by VAC", Color.yellow, false);
					}
					else if (rejection == ESteamRejection.AUTH_PUB_BAN)
					{
						ChatManager.say(Provider.pending[i].playerID.playerName + " was banned by BattlEye", Color.yellow, false);
					}
					if (Provider.pending[i].inventoryResult != SteamInventoryResult_t.Invalid)
					{
						SteamGameServerInventory.DestroyResult(Provider.pending[i].inventoryResult);
						Provider.pending[i].inventoryResult = SteamInventoryResult_t.Invalid;
					}
					Provider.pending.RemoveAt(i);
					if (i == 0)
					{
						Provider.verifyNextPlayerInQueue();
						break;
					}
					break;
				}
				else
				{
					i++;
				}
			}
			SteamGameServer.EndAuthSession(csteamID);
			NetMessages.SendMessageToClient(EClientMessage.Rejected, ENetReliability.Reliable, transportConnection, delegate(NetPakWriter writer)
			{
				writer.WriteEnum(rejection);
				writer.WriteString(explanation, 11);
			});
			transportConnection.CloseConnection();
		}

		// Token: 0x060036E3 RID: 14051 RVA: 0x00110424 File Offset: 0x0010E624
		[Obsolete]
		internal static void notifyClientPending(ITransportConnection transportConnection)
		{
		}

		// Token: 0x060036E4 RID: 14052 RVA: 0x00110428 File Offset: 0x0010E628
		private static bool findClientForKickBanDismiss(CSteamID steamID, out SteamPlayer foundClient, out byte foundIndex)
		{
			byte b = 0;
			while ((int)b < Provider.clients.Count)
			{
				SteamPlayer steamPlayer = Provider.clients[(int)b];
				if (steamPlayer.playerID.steamID == steamID)
				{
					foundClient = steamPlayer;
					foundIndex = b;
					return true;
				}
				b += 1;
			}
			foundClient = null;
			foundIndex = 0;
			return false;
		}

		// Token: 0x060036E5 RID: 14053 RVA: 0x00110479 File Offset: 0x0010E679
		private static void validateDisconnectedMaintainedIndex(CSteamID steamID, byte index)
		{
			if ((int)index >= Provider.clients.Count || Provider.clients[(int)index].playerID.steamID != steamID)
			{
				UnturnedLog.error("Clients array was modified during onServerDisconnected!");
			}
		}

		/// <summary>
		/// Notify client that they were kicked.
		/// </summary>
		// Token: 0x060036E6 RID: 14054 RVA: 0x001104B0 File Offset: 0x0010E6B0
		private static void notifyKickedInternal(ITransportConnection transportConnection, string reason)
		{
			NetMessages.SendMessageToClient(EClientMessage.Kicked, ENetReliability.Reliable, transportConnection, delegate(NetPakWriter writer)
			{
				writer.WriteString(reason, 11);
			});
		}

		// Token: 0x060036E7 RID: 14055 RVA: 0x001104E0 File Offset: 0x0010E6E0
		public static void kick(CSteamID steamID, string reason)
		{
			SteamPlayer steamPlayer;
			byte b;
			if (!Provider.findClientForKickBanDismiss(steamID, out steamPlayer, out b))
			{
				return;
			}
			UnturnedLog.info(string.Format("Kicking player {0} because \"{1}\"", steamID, reason));
			Provider.notifyKickedInternal(steamPlayer.transportConnection, reason);
			Provider.broadcastServerDisconnected(steamID);
			Provider.validateDisconnectedMaintainedIndex(steamID, b);
			SteamGameServer.EndAuthSession(steamID);
			Provider.removePlayer(b);
			Provider.replicateRemovePlayer(steamID, b);
		}

		/// <summary>
		/// Notify client that they were banned.
		/// </summary>
		// Token: 0x060036E8 RID: 14056 RVA: 0x0011053C File Offset: 0x0010E73C
		internal static void notifyBannedInternal(ITransportConnection transportConnection, string reason, uint duration)
		{
			NetMessages.SendMessageToClient(EClientMessage.Banned, ENetReliability.Reliable, transportConnection, delegate(NetPakWriter writer)
			{
				writer.WriteString(reason, 11);
				writer.WriteUInt32(duration);
			});
		}

		// Token: 0x060036E9 RID: 14057 RVA: 0x00110574 File Offset: 0x0010E774
		public static void ban(CSteamID steamID, string reason, uint duration)
		{
			SteamPlayer steamPlayer;
			byte b;
			if (!Provider.findClientForKickBanDismiss(steamID, out steamPlayer, out b))
			{
				return;
			}
			UnturnedLog.info(string.Format("Banning player {0} for {1} because \"{2}\"", steamID, TimeSpan.FromSeconds(duration), reason));
			Provider.notifyBannedInternal(steamPlayer.transportConnection, reason, duration);
			Provider.broadcastServerDisconnected(steamID);
			Provider.validateDisconnectedMaintainedIndex(steamID, b);
			SteamGameServer.EndAuthSession(steamID);
			Provider.removePlayer(b);
			Provider.replicateRemovePlayer(steamID, b);
		}

		/// <summary>
		/// Player left server by canceling their ticket, or we are disconnecting them without telling them.
		/// Does not send any packets to the disconnecting player.
		/// </summary>
		// Token: 0x060036EA RID: 14058 RVA: 0x001105E0 File Offset: 0x0010E7E0
		public static void dismiss(CSteamID steamID)
		{
			SteamPlayer steamPlayer;
			byte b;
			if (!Provider.findClientForKickBanDismiss(steamID, out steamPlayer, out b))
			{
				return;
			}
			Provider.broadcastServerDisconnected(steamID);
			Provider.validateDisconnectedMaintainedIndex(steamID, b);
			SteamGameServer.EndAuthSession(steamID);
			if (CommandWindow.shouldLogJoinLeave)
			{
				CommandWindow.Log(Provider.localization.format("PlayerDisconnectedText", steamID, steamPlayer.playerID.playerName, steamPlayer.playerID.characterName));
			}
			else
			{
				UnturnedLog.info(Provider.localization.format("PlayerDisconnectedText", steamID, steamPlayer.playerID.playerName, steamPlayer.playerID.characterName));
			}
			Provider.removePlayer(b);
			Provider.replicateRemovePlayer(steamID, b);
		}

		/// <summary>
		/// Callback when a pending player or existing player unexpectedly loses connection at the transport level.
		/// </summary>
		// Token: 0x060036EB RID: 14059 RVA: 0x00110684 File Offset: 0x0010E884
		private static void OnServerTransportConnectionFailure(ITransportConnection transportConnection, string debugString, bool isError)
		{
			SteamPending steamPending = Provider.findPendingPlayer(transportConnection);
			if (steamPending != null)
			{
				if (isError)
				{
					Provider.steam.clientsKickedForTransportConnectionFailureCount++;
					UnturnedLog.info(string.Format("Removing player in queue {0} due to transport failure ({1}) queue state: \"{2}\"", transportConnection, debugString, steamPending.GetQueueStateDebugString()));
				}
				else
				{
					UnturnedLog.info(string.Format("Removing player in queue {0} because they disconnected ({1}) queue state: \"{2}\"", transportConnection, debugString, steamPending.GetQueueStateDebugString()));
				}
				Provider.reject(transportConnection, ESteamRejection.LATE_PENDING);
				return;
			}
			SteamPlayer steamPlayer = Provider.findPlayer(transportConnection);
			if (steamPlayer != null)
			{
				if (isError)
				{
					Provider.steam.clientsKickedForTransportConnectionFailureCount++;
					UnturnedLog.info(string.Format("Removing player {0} due to transport failure ({1})", transportConnection, debugString));
				}
				else
				{
					UnturnedLog.info(string.Format("Removing player {0} because they disconnected ({1})", transportConnection, debugString));
				}
				Provider.dismiss(steamPlayer.playerID.steamID);
			}
		}

		// Token: 0x060036EC RID: 14060 RVA: 0x0011073C File Offset: 0x0010E93C
		internal static bool verifyTicket(CSteamID steamID, byte[] ticket)
		{
			return SteamGameServer.BeginAuthSession(ticket, ticket.Length, steamID) == EBeginAuthSessionResult.k_EBeginAuthSessionResultOK;
		}

		// Token: 0x060036ED RID: 14061 RVA: 0x0011074C File Offset: 0x0010E94C
		private static void openGameServer()
		{
			if (Provider.isServer || Provider.isClient)
			{
				UnturnedLog.error("Failed to open game server: session already in progress.");
				return;
			}
			ESecurityMode esecurityMode = ESecurityMode.LAN;
			ESteamServerVisibility serverVisibility = Dedicator.serverVisibility;
			if (serverVisibility != ESteamServerVisibility.Internet)
			{
				if (serverVisibility == ESteamServerVisibility.LAN)
				{
					esecurityMode = ESecurityMode.LAN;
				}
			}
			else if (Provider.configData.Server.VAC_Secure)
			{
				esecurityMode = ESecurityMode.SECURE;
			}
			else
			{
				esecurityMode = ESecurityMode.INSECURE;
			}
			if (esecurityMode == ESecurityMode.INSECURE)
			{
				CommandWindow.LogWarning(Provider.localization.format("InsecureWarningText"));
			}
			Provider.isVacActive = (esecurityMode == ESecurityMode.SECURE);
			if (Provider.IsBattlEyeEnabled && esecurityMode == ESecurityMode.SECURE)
			{
				if (!Provider.initializeBattlEyeServer())
				{
					Provider.QuitGame("BattlEye server init failed");
					return;
				}
				Provider.isBattlEyeActive = true;
			}
			else
			{
				Provider.isBattlEyeActive = false;
			}
			Provider.hasSetIsBattlEyeActive = true;
			bool flag = !Dedicator.offlineOnly;
			if (flag)
			{
				Provider.provider.multiplayerService.serverMultiplayerService.ready += Provider.handleServerReady;
			}
			try
			{
				Provider.provider.multiplayerService.serverMultiplayerService.open(Provider.ip, Provider.port, esecurityMode);
			}
			catch (Exception ex)
			{
				Provider.QuitGame("server init failed (" + ex.Message + ")");
				return;
			}
			Provider.serverTransport = NetTransportFactory.CreateServerTransport();
			UnturnedLog.info("Initializing {0}", new object[]
			{
				Provider.serverTransport.GetType().Name
			});
			Provider.serverTransport.Initialize(new ServerTransportConnectionFailureCallback(Provider.OnServerTransportConnectionFailure));
			Provider.backendRealtimeSeconds = SteamGameServerUtils.GetServerRealTime();
			Provider.authorityHoliday = (Provider._modeConfigData.Gameplay.Allow_Holidays ? HolidayUtil.BackendGetActiveHoliday() : ENPCHoliday.NONE);
			if (flag)
			{
				CommandWindow.Log("Waiting for Steam servers...");
				return;
			}
			Provider.initializeDedicatedUGC();
		}

		// Token: 0x060036EE RID: 14062 RVA: 0x001108F0 File Offset: 0x0010EAF0
		private static void closeGameServer()
		{
			if (!Provider.isServer)
			{
				UnturnedLog.error("Failed to close game server: no session in progress.");
				return;
			}
			Provider.broadcastServerShutdown();
			Provider._isServer = false;
			Provider.provider.multiplayerService.serverMultiplayerService.close();
		}

		/// <summary>
		/// Check whether a server is one of our favorites or not.
		/// </summary>
		// Token: 0x060036EF RID: 14063 RVA: 0x00110924 File Offset: 0x0010EB24
		public static bool GetServerIsFavorited(uint ip, ushort queryPort)
		{
			foreach (Provider.CachedFavorite cachedFavorite in Provider.cachedFavorites)
			{
				if (cachedFavorite.matchesServer(ip, queryPort))
				{
					return cachedFavorite.isFavorited;
				}
			}
			for (int i = 0; i < SteamMatchmaking.GetFavoriteGameCount(); i++)
			{
				AppId_t appId_t;
				uint num;
				ushort num2;
				ushort num3;
				uint num4;
				uint num5;
				SteamMatchmaking.GetFavoriteGame(i, out appId_t, out num, out num2, out num3, out num4, out num5);
				if ((num4 | Provider.STEAM_FAVORITE_FLAG_FAVORITE) == num4 && num == ip && num3 == queryPort)
				{
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// Set whether a server is one of our favorites or not.
		/// </summary>
		// Token: 0x060036F0 RID: 14064 RVA: 0x001109C4 File Offset: 0x0010EBC4
		public static void SetServerIsFavorited(uint ip, ushort connectionPort, ushort queryPort, bool newFavorited)
		{
			bool flag = false;
			foreach (Provider.CachedFavorite cachedFavorite in Provider.cachedFavorites)
			{
				if (cachedFavorite.matchesServer(ip, queryPort))
				{
					cachedFavorite.isFavorited = newFavorited;
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				Provider.CachedFavorite cachedFavorite2 = new Provider.CachedFavorite();
				cachedFavorite2.ip = ip;
				cachedFavorite2.queryPort = Provider.port;
				cachedFavorite2.isFavorited = newFavorited;
				Provider.cachedFavorites.Add(cachedFavorite2);
			}
			if (newFavorited)
			{
				SteamMatchmaking.AddFavoriteGame(Provider.APP_ID, ip, connectionPort, queryPort, Provider.STEAM_FAVORITE_FLAG_FAVORITE, SteamUtils.GetServerRealTime());
				UnturnedLog.info(string.Format("Added favorite server IP: {0} Connection Port: {1} Query Port: {2}", new IPv4Address(ip), connectionPort, queryPort));
				return;
			}
			SteamMatchmaking.RemoveFavoriteGame(Provider.APP_ID, ip, connectionPort, queryPort, Provider.STEAM_FAVORITE_FLAG_FAVORITE);
			UnturnedLog.info(string.Format("Removed favorite server IP: {0} Connection Port: {1} Query Port: {2}", new IPv4Address(ip), connectionPort, queryPort));
		}

		/// <summary>
		/// Open URL in the steam overlay, or if disabled use the default browser instead.
		/// Warning: any third party url should be checked by WebUtils.ParseThirdPartyUrl.
		/// </summary>
		// Token: 0x060036F1 RID: 14065 RVA: 0x00110AD0 File Offset: 0x0010ECD0
		public static void openURL(string url)
		{
			if (SteamUtils.IsOverlayEnabled())
			{
				SteamFriends.ActivateGameOverlayToWebPage(url, EActivateGameOverlayToWebPageMode.k_EActivateGameOverlayToWebPageMode_Default);
				return;
			}
			Process.Start(url);
		}

		/// <summary>
		/// Steam's favorites list requires that we know the server's IPv4 address and port,
		/// so we can't favorite when joining by Steam ID.
		/// </summary>
		// Token: 0x170009A4 RID: 2468
		// (get) Token: 0x060036F2 RID: 14066 RVA: 0x00110AE8 File Offset: 0x0010ECE8
		public static bool CanFavoriteCurrentServer
		{
			get
			{
				IPv4Address pv4Address;
				ushort num;
				ushort num2;
				return !Provider.isServer && Provider.clientTransport != null && (Provider.clientTransport.TryGetIPv4Address(out pv4Address) & Provider.clientTransport.TryGetConnectionPort(out num) & Provider.clientTransport.TryGetQueryPort(out num2));
			}
		}

		// Token: 0x170009A5 RID: 2469
		// (get) Token: 0x060036F3 RID: 14067 RVA: 0x00110B2C File Offset: 0x0010ED2C
		public static bool isCurrentServerFavorited
		{
			get
			{
				if (Provider.isServer || Provider.clientTransport == null)
				{
					return false;
				}
				IPv4Address pv4Address;
				Provider.clientTransport.TryGetIPv4Address(out pv4Address);
				ushort queryPort;
				Provider.clientTransport.TryGetQueryPort(out queryPort);
				return Provider.GetServerIsFavorited(pv4Address.value, queryPort);
			}
		}

		/// <summary>
		/// Toggle whether we've favorited the server we're currently playing on.
		/// </summary>
		// Token: 0x060036F4 RID: 14068 RVA: 0x00110B70 File Offset: 0x0010ED70
		public static void toggleCurrentServerFavorited()
		{
			if (Provider.isServer || Provider.clientTransport == null)
			{
				return;
			}
			IPv4Address pv4Address;
			ushort connectionPort;
			ushort queryPort;
			if (Provider.clientTransport.TryGetIPv4Address(out pv4Address) & Provider.clientTransport.TryGetConnectionPort(out connectionPort) & Provider.clientTransport.TryGetQueryPort(out queryPort))
			{
				bool newFavorited = !Provider.GetServerIsFavorited(pv4Address.value, queryPort);
				Provider.SetServerIsFavorited(pv4Address.value, connectionPort, queryPort, newFavorited);
				return;
			}
			UnturnedLog.info("Unable to toggle server favorite because connection details are unavailable");
		}

		// Token: 0x060036F5 RID: 14069 RVA: 0x00110BE0 File Offset: 0x0010EDE0
		private static void broadcastEnemyConnected(SteamPlayer player)
		{
			try
			{
				Provider.EnemyConnected enemyConnected = Provider.onEnemyConnected;
				if (enemyConnected != null)
				{
					enemyConnected(player);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Exception during onEnemyConnected:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036F6 RID: 14070 RVA: 0x00110C24 File Offset: 0x0010EE24
		private static void broadcastEnemyDisconnected(SteamPlayer player)
		{
			try
			{
				Provider.EnemyDisconnected enemyDisconnected = Provider.onEnemyDisconnected;
				if (enemyDisconnected != null)
				{
					enemyDisconnected(player);
				}
			}
			catch (Exception e)
			{
				UnturnedLog.warn("Exception during onEnemyDisconnected:");
				UnturnedLog.exception(e);
			}
		}

		// Token: 0x060036F7 RID: 14071 RVA: 0x00110C68 File Offset: 0x0010EE68
		private static void onPersonaStateChange(PersonaStateChange_t callback)
		{
			if (callback.m_nChangeFlags == EPersonaChange.k_EPersonaChangeName && callback.m_ulSteamID == Provider.client.m_SteamID)
			{
				Provider._clientName = SteamFriends.GetPersonaName();
			}
		}

		// Token: 0x060036F8 RID: 14072 RVA: 0x00110C90 File Offset: 0x0010EE90
		private static void onGameServerChangeRequested(GameServerChangeRequested_t callback)
		{
			if (Provider.isConnected)
			{
				return;
			}
			UnturnedLog.info("onGameServerChangeRequested {0} {1}", new object[]
			{
				callback.m_rgchServer,
				callback.m_rgchPassword
			});
			SteamConnectionInfo steamConnectionInfo = new SteamConnectionInfo(callback.m_rgchServer, callback.m_rgchPassword);
			UnturnedLog.info("External connect IP: {0} Port: {1} Password: '{2}'", new object[]
			{
				Parser.getIPFromUInt32(steamConnectionInfo.ip),
				steamConnectionInfo.port,
				steamConnectionInfo.password
			});
			MenuPlayConnectUI.connect(steamConnectionInfo, false);
		}

		// Token: 0x060036F9 RID: 14073 RVA: 0x00110D1C File Offset: 0x0010EF1C
		private static void onGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t callback)
		{
			if (Provider.isConnected)
			{
				return;
			}
			UnturnedLog.info("onGameRichPresenceJoinRequested {0}", new object[]
			{
				callback.m_rgchConnect
			});
			uint newIP;
			ushort newPort;
			string newPassword;
			if (CommandLine.TryGetSteamConnect(callback.m_rgchConnect, out newIP, out newPort, out newPassword))
			{
				SteamConnectionInfo steamConnectionInfo = new SteamConnectionInfo(newIP, newPort, newPassword);
				UnturnedLog.info("Rich presence connect IP: {0} Port: {1} Password: '{2}'", new object[]
				{
					Parser.getIPFromUInt32(steamConnectionInfo.ip),
					steamConnectionInfo.port,
					steamConnectionInfo.password
				});
				MenuPlayConnectUI.connect(steamConnectionInfo, false);
			}
		}

		// Token: 0x170009A6 RID: 2470
		// (get) Token: 0x060036FA RID: 14074 RVA: 0x00110DA4 File Offset: 0x0010EFA4
		// (set) Token: 0x060036FB RID: 14075 RVA: 0x00110DAB File Offset: 0x0010EFAB
		public static float timeLastPacketWasReceivedFromServer { get; internal set; }

		// Token: 0x170009A7 RID: 2471
		// (get) Token: 0x060036FC RID: 14076 RVA: 0x00110DB3 File Offset: 0x0010EFB3
		public static float ping
		{
			get
			{
				return Provider._ping;
			}
		}

		// Token: 0x060036FD RID: 14077 RVA: 0x00110DBC File Offset: 0x0010EFBC
		internal static void lag(float value)
		{
			value = Mathf.Clamp01(value);
			Provider._ping = value;
			for (int i = Provider.pings.Length - 1; i > 0; i--)
			{
				Provider.pings[i] = Provider.pings[i - 1];
				if (Provider.pings[i] > 0.001f)
				{
					Provider._ping += Provider.pings[i];
				}
			}
			Provider._ping /= (float)Provider.pings.Length;
			Provider.pings[0] = value;
		}

		// Token: 0x060036FE RID: 14078 RVA: 0x00110E38 File Offset: 0x0010F038
		internal static byte[] openTicket(SteamNetworkingIdentity serverIdentity)
		{
			if (Provider.ticketHandle != HAuthTicket.Invalid)
			{
				return null;
			}
			byte[] array = new byte[1024];
			string str;
			serverIdentity.ToString(out str);
			UnturnedLog.info("Calling GetAuthSessionTicket with identity " + str);
			uint num;
			Provider.ticketHandle = SteamUser.GetAuthSessionTicket(array, array.Length, out num, ref serverIdentity);
			if (num == 0U)
			{
				UnturnedLog.info("GetAuthSessionTicket returned size zero");
				return null;
			}
			UnturnedLog.info(string.Format("GetAuthSessionTicket ticket handle is valid: {0} (size: {1})", Provider.ticketHandle != HAuthTicket.Invalid, num));
			byte[] array2 = new byte[num];
			Buffer.BlockCopy(array, 0, array2, 0, (int)num);
			return array2;
		}

		// Token: 0x060036FF RID: 14079 RVA: 0x00110ED8 File Offset: 0x0010F0D8
		private static void closeTicket()
		{
			if (Provider.ticketHandle == HAuthTicket.Invalid)
			{
				return;
			}
			SteamUser.CancelAuthTicket(Provider.ticketHandle);
			Provider.ticketHandle = HAuthTicket.Invalid;
			UnturnedLog.info("Cancelled auth ticket");
		}

		// Token: 0x170009A8 RID: 2472
		// (get) Token: 0x06003700 RID: 14080 RVA: 0x00110F0A File Offset: 0x0010F10A
		// (set) Token: 0x06003701 RID: 14081 RVA: 0x00110F11 File Offset: 0x0010F111
		public static IProvider provider { get; protected set; }

		// Token: 0x170009A9 RID: 2473
		// (get) Token: 0x06003702 RID: 14082 RVA: 0x00110F19 File Offset: 0x0010F119
		public static bool isInitialized
		{
			get
			{
				return Provider._isInitialized;
			}
		}

		// Token: 0x170009AA RID: 2474
		// (get) Token: 0x06003703 RID: 14083 RVA: 0x00110F20 File Offset: 0x0010F120
		// (set) Token: 0x06003704 RID: 14084 RVA: 0x00110F36 File Offset: 0x0010F136
		public static uint time
		{
			get
			{
				return Provider._time + (uint)(Time.realtimeSinceStartup - Provider.timeOffset);
			}
			set
			{
				Provider._time = value;
				Provider.timeOffset = (uint)Time.realtimeSinceStartup;
			}
		}

		/// <summary>
		/// Number of seconds since January 1st, 1970 GMT as reported by backend servers.
		/// Used by holiday events to keep timing somewhat synced between players.
		/// </summary>
		// Token: 0x170009AB RID: 2475
		// (get) Token: 0x06003705 RID: 14085 RVA: 0x00110F49 File Offset: 0x0010F149
		// (set) Token: 0x06003706 RID: 14086 RVA: 0x00110F5D File Offset: 0x0010F15D
		public static uint backendRealtimeSeconds
		{
			get
			{
				return Provider.initialBackendRealtimeSeconds + (uint)(Time.realtimeSinceStartup - Provider.initialLocalRealtime);
			}
			private set
			{
				Provider.initialBackendRealtimeSeconds = value;
				Provider.initialLocalRealtime = Time.realtimeSinceStartup;
				Provider.BackendRealtimeAvailableHandler backendRealtimeAvailableHandler = Provider.onBackendRealtimeAvailable;
				if (backendRealtimeAvailableHandler == null)
				{
					return;
				}
				backendRealtimeAvailableHandler();
			}
		}

		// Token: 0x170009AC RID: 2476
		// (get) Token: 0x06003707 RID: 14087 RVA: 0x00110F7E File Offset: 0x0010F17E
		public static DateTime backendRealtimeDate
		{
			get
			{
				return Provider.unixEpochDateTime.AddSeconds(Provider.backendRealtimeSeconds);
			}
		}

		/// <summary>
		/// Has the initial backend realtime been queried yet?
		/// Not available immediately on servers because SteamGameServerUtils cannot be used until the actual Steam instance is available.
		/// </summary>
		// Token: 0x170009AD RID: 2477
		// (get) Token: 0x06003708 RID: 14088 RVA: 0x00110F91 File Offset: 0x0010F191
		public static bool isBackendRealtimeAvailable
		{
			get
			{
				return Provider.initialBackendRealtimeSeconds > 0U;
			}
		}

		// Token: 0x06003709 RID: 14089 RVA: 0x00110F9B File Offset: 0x0010F19B
		private IEnumerator QuitAfterDelay(float seconds)
		{
			yield return new WaitForSeconds(seconds);
			Application.quitting -= this.onApplicationQuitting;
			this.onApplicationQuitting();
			Provider.QuitGame("server shutdown");
			yield break;
		}

		// Token: 0x0600370A RID: 14090 RVA: 0x00110FB1 File Offset: 0x0010F1B1
		private static void onAPIWarningMessage(int severity, StringBuilder warning)
		{
			CommandWindow.LogWarning("Steam API Warning Message:");
			CommandWindow.LogWarning("Severity: " + severity.ToString());
			CommandWindow.LogWarning("Warning: " + ((warning != null) ? warning.ToString() : null));
		}

		// Token: 0x0600370B RID: 14091 RVA: 0x00110FF0 File Offset: 0x0010F1F0
		private void updateDebug()
		{
			Provider.debugUpdates++;
			if (Time.realtimeSinceStartup - Provider.debugLastUpdate > 1f)
			{
				Provider.debugUPS = (int)((float)Provider.debugUpdates / (Time.realtimeSinceStartup - Provider.debugLastUpdate));
				Provider.debugLastUpdate = Time.realtimeSinceStartup;
				Provider.debugUpdates = 0;
			}
		}

		// Token: 0x0600370C RID: 14092 RVA: 0x00111044 File Offset: 0x0010F244
		private void tickDebug()
		{
			Provider.debugTicks++;
			if (Time.realtimeSinceStartup - Provider.debugLastTick > 1f)
			{
				Provider.debugTPS = (int)((float)Provider.debugTicks / (Time.realtimeSinceStartup - Provider.debugLastTick));
				Provider.debugLastTick = Time.realtimeSinceStartup;
				Provider.debugTicks = 0;
			}
		}

		// Token: 0x0600370D RID: 14093 RVA: 0x00111097 File Offset: 0x0010F297
		private IEnumerator downloadIcon(Provider.PendingIconRequest iconQueryParams)
		{
			using (UnityWebRequest request = UnityWebRequestTexture.GetTexture(iconQueryParams.url, true))
			{
				request.timeout = 15;
				yield return request.SendWebRequest();
				Texture2D texture2D = null;
				bool flag = false;
				if (request.result != UnityWebRequest.Result.Success)
				{
					UnturnedLog.warn(string.Format("{0} downloading \"{1}\" for icon query: \"{2}\"", request.result, iconQueryParams.url, request.error));
				}
				else
				{
					Texture2D content = DownloadHandlerTexture.GetContent(request);
					content.hideFlags = HideFlags.HideAndDontSave;
					content.filterMode = FilterMode.Trilinear;
					if (iconQueryParams.shouldCache)
					{
						if (Provider.downloadedIconCache.TryGetValue(iconQueryParams.url, out texture2D))
						{
							UnityEngine.Object.Destroy(content);
						}
						else
						{
							Provider.downloadedIconCache.Add(iconQueryParams.url, content);
							texture2D = content;
						}
						flag = false;
					}
					else
					{
						texture2D = content;
						flag = true;
					}
				}
				if (iconQueryParams.callback == null)
				{
					if (flag && texture2D != null)
					{
						UnityEngine.Object.Destroy(texture2D);
					}
				}
				else
				{
					try
					{
						iconQueryParams.callback(texture2D, flag);
					}
					catch (Exception e)
					{
						UnturnedLog.exception(e, "Caught exception during texture downloaded callback:");
					}
				}
				if (iconQueryParams.shouldCache)
				{
					Provider.pendingCachableIconRequests.Remove(iconQueryParams.url);
				}
			}
			UnityWebRequest request = null;
			yield break;
			yield break;
		}

		// Token: 0x0600370E RID: 14094 RVA: 0x001110A8 File Offset: 0x0010F2A8
		public static void destroyCachedIcon(string url)
		{
			Texture2D obj;
			if (Provider.downloadedIconCache.TryGetValue(url, out obj))
			{
				UnityEngine.Object.Destroy(obj);
				Provider.downloadedIconCache.Remove(url);
			}
		}

		// Token: 0x0600370F RID: 14095 RVA: 0x001110D8 File Offset: 0x0010F2D8
		public static void refreshIcon(Provider.IconQueryParams iconQueryParams)
		{
			if (iconQueryParams.callback == null)
			{
				return;
			}
			if (string.IsNullOrEmpty(iconQueryParams.url) || !Provider.allowWebRequests)
			{
				iconQueryParams.callback(null, false);
				return;
			}
			iconQueryParams.url = iconQueryParams.url.Trim();
			if (string.IsNullOrEmpty(iconQueryParams.url))
			{
				iconQueryParams.callback(null, false);
				return;
			}
			if (iconQueryParams.shouldCache)
			{
				Texture2D icon;
				if (Provider.downloadedIconCache.TryGetValue(iconQueryParams.url, out icon))
				{
					iconQueryParams.callback(icon, false);
					return;
				}
				Provider.PendingIconRequest pendingIconRequest;
				if (Provider.pendingCachableIconRequests.TryGetValue(iconQueryParams.url, out pendingIconRequest))
				{
					Provider.PendingIconRequest pendingIconRequest2 = pendingIconRequest;
					pendingIconRequest2.callback = (Provider.IconQueryCallback)Delegate.Combine(pendingIconRequest2.callback, iconQueryParams.callback);
					return;
				}
			}
			Provider.PendingIconRequest pendingIconRequest3 = new Provider.PendingIconRequest();
			pendingIconRequest3.url = iconQueryParams.url;
			pendingIconRequest3.callback = iconQueryParams.callback;
			pendingIconRequest3.shouldCache = iconQueryParams.shouldCache;
			if (iconQueryParams.shouldCache)
			{
				Provider.pendingCachableIconRequests.Add(iconQueryParams.url, pendingIconRequest3);
			}
			Provider.steam.StartCoroutine(Provider.steam.downloadIcon(pendingIconRequest3));
		}

		// Token: 0x06003710 RID: 14096 RVA: 0x001111F8 File Offset: 0x0010F3F8
		private void Update()
		{
			if (!Provider.isInitialized)
			{
				return;
			}
			if (Time.unscaledDeltaTime > 1.5f)
			{
				UnturnedLog.info("Long delay between Updates: {0}s", new object[]
				{
					Time.unscaledDeltaTime
				});
			}
			if (Provider.battlEyeClientHandle != IntPtr.Zero && Provider.battlEyeClientRunData != null && Provider.battlEyeClientRunData.pfnRun != null)
			{
				Provider.battlEyeClientRunData.pfnRun();
			}
			if (Provider.battlEyeServerHandle != IntPtr.Zero && Provider.battlEyeServerRunData != null && Provider.battlEyeServerRunData.pfnRun != null)
			{
				Provider.battlEyeServerRunData.pfnRun();
			}
			if (Provider.isConnected)
			{
				Provider.listen();
			}
			this.updateDebug();
			Provider.provider.update();
			if (Provider.countShutdownTimer > 0)
			{
				if (Time.realtimeSinceStartup - Provider.lastTimerMessage > 1f)
				{
					Provider.lastTimerMessage = Time.realtimeSinceStartup;
					Provider.countShutdownTimer--;
					if (Provider.countShutdownTimer == 300 || Provider.countShutdownTimer == 60 || Provider.countShutdownTimer == 30 || Provider.countShutdownTimer == 15 || Provider.countShutdownTimer == 3 || Provider.countShutdownTimer == 2 || Provider.countShutdownTimer == 1)
					{
						ChatManager.say(Provider.localization.format("Shutdown", Provider.countShutdownTimer), ChatManager.welcomeColor, false);
						return;
					}
				}
			}
			else if (Provider.countShutdownTimer == 0)
			{
				UnturnedLog.info("Server shutdown timer reached zero");
				Provider.didServerShutdownTimerReachZero = true;
				Provider.countShutdownTimer = -1;
				Provider.broadcastCommenceShutdown();
				bool flag = Provider._clients.Count > 0;
				if (Provider._clients.Count > 0)
				{
					NetMessages.SendMessageToClients(EClientMessage.Shutdown, ENetReliability.Reliable, Provider.GatherRemoteClientConnections(), delegate(NetPakWriter writer)
					{
						writer.WriteString(Provider.shutdownMessage, 11);
					});
				}
				foreach (SteamPlayer steamPlayer in Provider._clients)
				{
					SteamGameServer.EndAuthSession(steamPlayer.playerID.steamID);
				}
				float num = flag ? 1f : 0f;
				if (flag)
				{
					UnturnedLog.info(string.Format("Delaying server quit by {0}s to ensure shutdown message reaches clients", num));
				}
				base.StartCoroutine(this.QuitAfterDelay(num));
			}
		}

		// Token: 0x06003711 RID: 14097 RVA: 0x00111444 File Offset: 0x0010F644
		private void FixedUpdate()
		{
			if (!Provider.isInitialized)
			{
				return;
			}
			this.tickDebug();
		}

		/// <summary>
		/// In here because we want to call this very early in startup after initializing provider,
		/// but with plenty of time to hopefully install maps prior to reaching the main menu.
		/// </summary>
		// Token: 0x06003712 RID: 14098 RVA: 0x00111454 File Offset: 0x0010F654
		public static void initAutoSubscribeMaps()
		{
			if (Provider.statusData == null || Provider.statusData.Maps == null)
			{
				return;
			}
			foreach (AutoSubscribeMap autoSubscribeMap in Provider.statusData.Maps.Auto_Subscribe)
			{
				if (!LocalNews.hasAutoSubscribedToWorkshopItem(autoSubscribeMap.Workshop_File_Id) && new DateTimeRange(autoSubscribeMap.Start, autoSubscribeMap.End).isNowWithinRange())
				{
					LocalNews.markAutoSubscribedToWorkshopItem(autoSubscribeMap.Workshop_File_Id);
					Provider.provider.workshopService.setSubscribed(autoSubscribeMap.Workshop_File_Id, true);
				}
			}
			ConvenientSavedata.SaveIfDirty();
		}

		/// <summary>
		/// This file is of particular importance to the dedicated server because otherwise Steam networking sockets
		/// will say the certificate is for the wrong app. When launching the game outside Steam this sets the app.
		/// </summary>
		// Token: 0x06003713 RID: 14099 RVA: 0x00111508 File Offset: 0x0010F708
		private void WriteSteamAppIdFileAndEnvironmentVariables()
		{
			uint appId = Provider.APP_ID.m_AppId;
			string text = appId.ToString(CultureInfo.InvariantCulture);
			UnturnedLog.info("Unturned overriding Steam AppId with \"" + text + "\"");
			try
			{
				Environment.SetEnvironmentVariable("SteamOverlayGameId", text, EnvironmentVariableTarget.Process);
				Environment.SetEnvironmentVariable("SteamGameId", text, EnvironmentVariableTarget.Process);
				Environment.SetEnvironmentVariable("SteamAppId", text, EnvironmentVariableTarget.Process);
			}
			catch (Exception e)
			{
				UnturnedLog.exception(e, "Caught exception writing Steam environment variables:");
			}
			string path = PathEx.Join(UnityPaths.GameDirectory, "steam_appid.txt");
			try
			{
				using (FileStream fileStream = new FileStream(path, FileMode.OpenOrCreate, FileAccess.Write, FileShare.ReadWrite))
				{
					using (StreamWriter streamWriter = new StreamWriter(fileStream, Encoding.ASCII))
					{
						streamWriter.Write(text);
					}
				}
			}
			catch (Exception e2)
			{
				UnturnedLog.exception(e2, "Caught exception writing steam_appid.txt file:");
			}
		}

		/// <summary>
		/// Hackily exposed as an easy way for editor code to check the verison number.
		/// </summary>
		// Token: 0x06003714 RID: 14100 RVA: 0x00111600 File Offset: 0x0010F800
		public static StatusData LoadStatusData()
		{
			if (ReadWrite.fileExists("/Status.json", false, true))
			{
				try
				{
					return ReadWrite.deserializeJSON<StatusData>("/Status.json", false, true);
				}
				catch (Exception e)
				{
					UnturnedLog.exception(e, "Unable to parse Status.json! consider validating with a JSON linter");
				}
			}
			return null;
		}

		// Token: 0x06003715 RID: 14101 RVA: 0x0011164C File Offset: 0x0010F84C
		private void LoadPreferences()
		{
			string path = PathEx.Join(UnturnedPaths.RootDirectory, "Preferences.json");
			if (ReadWrite.fileExists(path, false, false))
			{
				try
				{
					Provider._preferenceData = ReadWrite.deserializeJSON<PreferenceData>(path, false, false);
				}
				catch (Exception e)
				{
					UnturnedLog.exception(e, "Unable to parse Preferences.json! consider validating with a JSON linter");
					Provider._preferenceData = null;
				}
				if (Provider.preferenceData == null)
				{
					Provider._preferenceData = new PreferenceData();
				}
			}
			else
			{
				Provider._preferenceData = new PreferenceData();
			}
			Provider._preferenceData.Viewmodel.Clamp();
			SleekCustomization.defaultTextContrast = Provider._preferenceData.Graphics.Default_Text_Contrast;
			SleekCustomization.inconspicuousTextContrast = Provider._preferenceData.Graphics.Inconspicuous_Text_Contrast;
			SleekCustomization.colorfulTextContrast = Provider._preferenceData.Graphics.Colorful_Text_Contrast;
			try
			{
				ReadWrite.serializeJSON<PreferenceData>(path, false, false, Provider.preferenceData);
			}
			catch (Exception e2)
			{
				UnturnedLog.exception(e2, "Caught exception re-serializing Preferences.json:");
			}
		}

		// Token: 0x06003716 RID: 14102 RVA: 0x00111734 File Offset: 0x0010F934
		public void awake()
		{
			Provider._statusData = Provider.LoadStatusData();
			if (Provider.statusData == null)
			{
				Provider._statusData = new StatusData();
			}
			HolidayUtil.scheduleHolidays(Provider.statusData.Holidays);
			Provider.APP_VERSION = Provider.statusData.Game.FormatApplicationVersion();
			Provider.APP_VERSION_PACKED = Parser.getUInt32FromIP(Provider.APP_VERSION);
			if (Provider.isInitialized)
			{
				UnityEngine.Object.Destroy(base.gameObject);
				return;
			}
			Provider._isInitialized = true;
			UnityEngine.Object.DontDestroyOnLoad(base.gameObject);
			Provider.steam = this;
			Level.onLevelLoaded = (LevelLoaded)Delegate.Combine(Level.onLevelLoaded, new LevelLoaded(Provider.onLevelLoaded));
			Application.quitting += this.onApplicationQuitting;
			Application.wantsToQuit += this.onApplicationWantsToQuit;
			if (Dedicator.IsDedicatedServer)
			{
				try
				{
					this.WriteSteamAppIdFileAndEnvironmentVariables();
					Provider.provider = new SteamworksProvider(new SteamworksAppInfo(Provider.APP_ID.m_AppId, Provider.APP_NAME, Provider.APP_VERSION, true));
					Provider.provider.intialize();
				}
				catch (Exception ex)
				{
					Provider.QuitGame("Steam init exception (" + ex.Message + ")");
					return;
				}
				string language;
				if (!CommandLine.tryGetLanguage(out language, out Provider._path))
				{
					Provider._path = ReadWrite.PATH + "/Localization/";
					language = "English";
				}
				Provider.language = language;
				Provider.localizationRoot = Provider.path + Provider.language;
				Provider.localization = Localization.read("/Server/ServerConsole.dat");
				Provider.p2pSessionConnectFail = Callback<P2PSessionConnectFail_t>.CreateGameServer(new Callback<P2PSessionConnectFail_t>.DispatchDelegate(Provider.onP2PSessionConnectFail));
				Provider.validateAuthTicketResponse = Callback<ValidateAuthTicketResponse_t>.CreateGameServer(new Callback<ValidateAuthTicketResponse_t>.DispatchDelegate(Provider.onValidateAuthTicketResponse));
				Provider.clientGroupStatus = Callback<GSClientGroupStatus_t>.CreateGameServer(new Callback<GSClientGroupStatus_t>.DispatchDelegate(Provider.onClientGroupStatus));
				Provider._isPro = true;
				CommandWindow.Log("Game version: " + Provider.APP_VERSION + " Engine version: " + Application.unityVersion);
				Provider.maxPlayers = 8;
				Provider.queueSize = 8;
				Provider.serverName = "Unturned";
				Provider.serverPassword = "";
				Provider.ip = 0U;
				Provider.port = 27015;
				Provider.map = "PEI";
				Provider.isPvP = true;
				Provider.isWhitelisted = false;
				Provider.hideAdmins = false;
				Provider.hasCheats = false;
				Provider.filterName = false;
				Provider.mode = EGameMode.NORMAL;
				Provider.isGold = false;
				Provider.gameMode = null;
				Provider.cameraMode = ECameraMode.FIRST;
				Commander.init();
				SteamWhitelist.load();
				SteamBlacklist.load();
				SteamAdminlist.load();
				string[] commands = CommandLine.getCommands();
				UnturnedLog.info(string.Format("Executing {0} potential game command(s) from the command-line:", commands.Length));
				for (int i = 0; i < commands.Length; i++)
				{
					if (!Commander.execute(CSteamID.Nil, commands[i]))
					{
						UnturnedLog.info("Did not match \"" + commands[i] + "\" with any commands");
					}
				}
				if (ServerSavedata.fileExists("/Server/Commands.dat"))
				{
					FileStream fileStream = null;
					StreamReader streamReader = null;
					try
					{
						fileStream = new FileStream(ReadWrite.PATH + "/Servers/" + Provider.serverID + "/Server/Commands.dat", FileMode.Open, FileAccess.Read, FileShare.Read);
						streamReader = new StreamReader(fileStream);
						string text;
						while ((text = streamReader.ReadLine()) != null)
						{
							if (!string.IsNullOrWhiteSpace(text) && !Commander.execute(CSteamID.Nil, text))
							{
								UnturnedLog.error("Unknown entry in Commands.dat: '{0}'", new object[]
								{
									text
								});
							}
						}
						goto IL_352;
					}
					finally
					{
						if (fileStream != null)
						{
							fileStream.Close();
						}
						if (streamReader != null)
						{
							streamReader.Close();
						}
					}
				}
				Data data = new Data();
				ServerSavedata.writeData("/Server/Commands.dat", data);
				IL_352:
				if (!ServerSavedata.folderExists("/Bundles"))
				{
					ServerSavedata.createFolder("/Bundles");
				}
				if (!ServerSavedata.folderExists("/Maps"))
				{
					ServerSavedata.createFolder("/Maps");
				}
				if (!ServerSavedata.folderExists("/Workshop/Content"))
				{
					ServerSavedata.createFolder("/Workshop/Content");
				}
				if (!ServerSavedata.folderExists("/Workshop/Maps"))
				{
					ServerSavedata.createFolder("/Workshop/Maps");
				}
				Provider._configData = ConfigData.CreateDefault(false);
				if (ServerSavedata.fileExists("/Config.json"))
				{
					try
					{
						ServerSavedata.populateJSON("/Config.json", Provider._configData);
					}
					catch (Exception e)
					{
						UnturnedLog.error("Exception while parsing server config:");
						UnturnedLog.exception(e);
					}
				}
				ServerSavedata.serializeJSON<ConfigData>("/Config.json", Provider.configData);
				Provider._modeConfigData = Provider._configData.getModeConfig(Provider.mode);
				if (Provider._modeConfigData == null)
				{
					Provider._modeConfigData = new ModeConfigData(Provider.mode);
				}
				if (!Dedicator.offlineOnly)
				{
					HostBansManager.Get().Refresh();
				}
				this.LogSystemInfo();
				return;
			}
			try
			{
				this.WriteSteamAppIdFileAndEnvironmentVariables();
				Provider.provider = new SteamworksProvider(new SteamworksAppInfo(Provider.APP_ID.m_AppId, Provider.APP_NAME, Provider.APP_VERSION, false));
				Provider.provider.intialize();
			}
			catch (Exception ex2)
			{
				Provider.QuitGame("Steam init exception (" + ex2.Message + ")");
				return;
			}
			Provider.backendRealtimeSeconds = SteamUtils.GetServerRealTime();
			Provider.authorityHoliday = HolidayUtil.BackendGetActiveHoliday();
			Provider.apiWarningMessageHook = new SteamAPIWarningMessageHook_t(Provider.onAPIWarningMessage);
			SteamUtils.SetWarningMessageHook(Provider.apiWarningMessageHook);
			Provider.screenshotRequestedCallback = Callback<ScreenshotRequested_t>.Create(new Callback<ScreenshotRequested_t>.DispatchDelegate(Provider.OnSteamScreenshotRequested));
			SteamScreenshots.HookScreenshots(true);
			Provider.time = SteamUtils.GetServerRealTime();
			Provider.personaStateChange = Callback<PersonaStateChange_t>.Create(new Callback<PersonaStateChange_t>.DispatchDelegate(Provider.onPersonaStateChange));
			Provider.gameServerChangeRequested = Callback<GameServerChangeRequested_t>.Create(new Callback<GameServerChangeRequested_t>.DispatchDelegate(Provider.onGameServerChangeRequested));
			Provider.gameRichPresenceJoinRequested = Callback<GameRichPresenceJoinRequested_t>.Create(new Callback<GameRichPresenceJoinRequested_t>.DispatchDelegate(Provider.onGameRichPresenceJoinRequested));
			Provider._user = SteamUser.GetSteamID();
			Provider._client = Provider.user;
			Provider._clientHash = Hash.SHA1(Provider.client);
			Provider._clientName = SteamFriends.GetPersonaName();
			Provider.provider.statisticsService.userStatisticsService.requestStatistics();
			Provider.provider.statisticsService.globalStatisticsService.requestStatistics();
			Provider.provider.workshopService.refreshUGC();
			Provider.provider.workshopService.refreshPublished();
			if (Provider.shouldCheckForGoldUpgrade)
			{
				Provider._isPro = SteamApps.BIsSubscribedApp(Provider.PRO_ID);
			}
			UnturnedLog.info("Game version: " + Provider.APP_VERSION + " Engine version: " + Application.unityVersion);
			Provider.isLoadingInventory = true;
			Provider.provider.economyService.GrantPromoItems();
			SteamNetworkingSockets.InitAuthentication();
			if (SteamUser.BLoggedOn() && Provider.allowWebRequests)
			{
				HostBansManager.Get().Refresh();
			}
			LiveConfig.Refresh();
			ProfanityFilter.InitSteam();
			string language2;
			if (CommandLine.tryGetLanguage(out language2, out Provider._path))
			{
				Provider.language = language2;
				Provider.localizationRoot = Provider.path + Provider.language;
			}
			else
			{
				string steamUILanguage = SteamUtils.GetSteamUILanguage();
				Provider.language = steamUILanguage.Substring(0, 1).ToUpper() + steamUILanguage.Substring(1, steamUILanguage.Length - 1).ToLower();
				bool flag = false;
				foreach (SteamContent steamContent in Provider.provider.workshopService.ugc)
				{
					if (steamContent.type == ESteamUGCType.LOCALIZATION && ReadWrite.folderExists(steamContent.path + "/" + Provider.language, false))
					{
						Provider._path = steamContent.path + "/";
						Provider.localizationRoot = Provider.path + Provider.language;
						flag = true;
						UnturnedLog.info("Found Steam language '{0}' in workshop item {1}", new object[]
						{
							steamUILanguage,
							steamContent.publishedFileID
						});
						break;
					}
				}
				if (!flag && ReadWrite.folderExists("/Localization/" + Provider.language))
				{
					Provider._path = ReadWrite.PATH + "/Localization/";
					Provider.localizationRoot = Provider.path + Provider.language;
					flag = true;
					UnturnedLog.info("Found Steam language '{0}' in root Localization directory", new object[]
					{
						steamUILanguage
					});
				}
				if (!flag && ReadWrite.folderExists("/Sandbox/" + Provider.language))
				{
					Provider._path = ReadWrite.PATH + "/Sandbox/";
					Provider.localizationRoot = Provider.path + Provider.language;
					flag = true;
					UnturnedLog.info("Found Steam language '{0}' in Sandbox directory", new object[]
					{
						steamUILanguage
					});
				}
				if (!flag)
				{
					foreach (SteamContent steamContent2 in Provider.provider.workshopService.ugc)
					{
						bool flag2 = ReadWrite.folderExists(steamContent2.path + "/Editor", false);
						bool flag3 = ReadWrite.folderExists(steamContent2.path + "/Menu", false);
						bool flag4 = ReadWrite.folderExists(steamContent2.path + "/Player", false);
						bool flag5 = ReadWrite.folderExists(steamContent2.path + "/Server", false);
						bool flag6 = ReadWrite.folderExists(steamContent2.path + "/Shared", false);
						if (flag2 && flag3 && flag4 && flag5 && flag6)
						{
							Provider._path = null;
							Provider.localizationRoot = steamContent2.path;
							flag = true;
							UnturnedLog.info("Found language files for unknown language in workshop item {0}", new object[]
							{
								steamContent2.publishedFileID
							});
						}
					}
				}
				if (!flag)
				{
					Provider._path = ReadWrite.PATH + "/Localization/";
					Provider.language = "English";
					Provider.localizationRoot = Provider.path + Provider.language;
				}
			}
			Provider.provider.economyService.loadTranslationEconInfo();
			Provider.localization = Localization.read("/Server/ServerConsole.dat");
			Provider.updateRichPresence();
			Provider._configData = ConfigData.CreateDefault(true);
			Provider._modeConfigData = Provider.configData.Normal;
			this.LoadPreferences();
			if (ReadWrite.fileExists("/StreamerNames.json", false, true))
			{
				try
				{
					Provider.streamerNames = ReadWrite.deserializeJSON<List<string>>("/StreamerNames.json", false, true);
				}
				catch (Exception e2)
				{
					UnturnedLog.exception(e2, "Unable to parse StreamerNames.json! consider validating with a JSON linter");
					Provider.streamerNames = null;
				}
				if (Provider.streamerNames == null)
				{
					Provider.streamerNames = new List<string>();
				}
			}
			else
			{
				Provider.streamerNames = new List<string>();
			}
			this.LogSystemInfo();
		}

		// Token: 0x06003717 RID: 14103 RVA: 0x0011215C File Offset: 0x0011035C
		public void start()
		{
		}

		// Token: 0x06003718 RID: 14104 RVA: 0x00112160 File Offset: 0x00110360
		private void LogSystemInfo()
		{
			try
			{
				UnturnedLog.info("Platform: {0}", new object[]
				{
					Application.platform
				});
				UnturnedLog.info("Operating System: " + SystemInfo.operatingSystem);
				UnturnedLog.info("System Memory: " + SystemInfo.systemMemorySize.ToString() + "MB");
				UnturnedLog.info("Graphics Device Name: " + SystemInfo.graphicsDeviceName);
				UnturnedLog.info("Graphics Device Type: " + SystemInfo.graphicsDeviceType.ToString());
				UnturnedLog.info("Graphics Memory: " + SystemInfo.graphicsMemorySize.ToString() + "MB");
				UnturnedLog.info("Graphics Multi-Threaded: " + SystemInfo.graphicsMultiThreaded.ToString());
				UnturnedLog.info("Render Threading Mode: " + SystemInfo.renderingThreadingMode.ToString());
				UnturnedLog.info("Supports Audio: " + SystemInfo.supportsAudio.ToString());
				UnturnedLog.info("Supports Instancing: " + SystemInfo.supportsInstancing.ToString());
				UnturnedLog.info("Supports Motion Vectors: " + SystemInfo.supportsMotionVectors.ToString());
				UnturnedLog.info("Supports Ray Tracing: " + SystemInfo.supportsRayTracing.ToString());
			}
			catch (Exception e)
			{
				UnturnedLog.exception(e, "Caught exception while logging system info:");
			}
		}

		/// <summary>
		/// Has the onApplicationQuitting callback been invoked?
		/// </summary>
		// Token: 0x170009AE RID: 2478
		// (get) Token: 0x06003719 RID: 14105 RVA: 0x001122F0 File Offset: 0x001104F0
		// (set) Token: 0x0600371A RID: 14106 RVA: 0x001122F7 File Offset: 0x001104F7
		public static bool isApplicationQuitting { get; private set; }

		/// <summary>
		/// Moved from OnApplicationQuit when that was deprecated.
		/// </summary>
		// Token: 0x0600371B RID: 14107 RVA: 0x00112300 File Offset: 0x00110500
		private void onApplicationQuitting()
		{
			UnturnedLog.info("Application quitting");
			Provider.isApplicationQuitting = true;
			if (!Dedicator.IsDedicatedServer)
			{
				ConvenientSavedata.save();
				LocalPlayerBlocklist.SaveIfDirty();
			}
			if (!Provider.isInitialized)
			{
				return;
			}
			Provider.RequestDisconnect("application quitting");
			Provider.provider.shutdown();
			UnturnedLog.info("Finished quitting");
		}

		// Token: 0x0600371C RID: 14108 RVA: 0x00112354 File Offset: 0x00110554
		public static void QuitGame(string reason)
		{
			UnturnedLog.info("Quit game: " + reason);
			Provider.wasQuitGameCalled = true;
			Application.Quit();
		}

		/// <summary>
		/// Moved from OnApplicationQuit when Application.CancelQuit was deprecated.
		/// </summary>
		// Token: 0x0600371D RID: 14109 RVA: 0x00112374 File Offset: 0x00110574
		private bool onApplicationWantsToQuit()
		{
			return Provider.wasQuitGameCalled || Dedicator.IsDedicatedServer || Provider.isServer || !Provider.isPvP || Provider.clients.Count <= 1 || !(Player.player != null) || Player.player.movement.isSafe || !Player.player.life.IsAlive;
		}

		// Token: 0x04001FF1 RID: 8177
		public static readonly string STEAM_IC = "Steam";

		// Token: 0x04001FF2 RID: 8178
		public static readonly string STEAM_DC = "<color=#2784c6>Steam</color>";

		// Token: 0x04001FF3 RID: 8179
		public static readonly AppId_t APP_ID = new AppId_t(304930U);

		// Token: 0x04001FF4 RID: 8180
		public static readonly AppId_t PRO_ID = new AppId_t(306460U);

		// Token: 0x04001FF7 RID: 8183
		public static readonly string APP_NAME = "Unturned";

		// Token: 0x04001FF8 RID: 8184
		public static readonly string APP_AUTHOR = "Nelson Sexton";

		// Token: 0x04001FF9 RID: 8185
		public static readonly int CLIENT_TIMEOUT = 30;

		// Token: 0x04001FFA RID: 8186
		internal static readonly float PING_REQUEST_INTERVAL = 1f;

		// Token: 0x04001FFB RID: 8187
		private static bool isCapturingScreenshot;

		// Token: 0x04001FFC RID: 8188
		private static StaticResourceRef<Material> screenshotBlitMaterial = new StaticResourceRef<Material>("Materials/ScreenshotBlit");

		// Token: 0x04001FFD RID: 8189
		private static Callback<ScreenshotRequested_t> screenshotRequestedCallback;

		// Token: 0x04001FFE RID: 8190
		private static string privateLanguage;

		// Token: 0x04001FFF RID: 8191
		internal static bool languageIsEnglish;

		// Token: 0x04002000 RID: 8192
		private static string _path;

		// Token: 0x04002002 RID: 8194
		public static Local localization;

		// Token: 0x04002004 RID: 8196
		internal static IntPtr battlEyeClientHandle = IntPtr.Zero;

		// Token: 0x04002005 RID: 8197
		internal static BEClient.BECL_GAME_DATA battlEyeClientInitData = null;

		// Token: 0x04002006 RID: 8198
		internal static BEClient.BECL_BE_DATA battlEyeClientRunData = null;

		// Token: 0x04002007 RID: 8199
		private static bool battlEyeHasRequiredRestart = false;

		// Token: 0x04002008 RID: 8200
		internal static readonly NetLength battlEyeBufferSize = new NetLength(4095U);

		// Token: 0x04002009 RID: 8201
		internal static IntPtr battlEyeServerHandle = IntPtr.Zero;

		// Token: 0x0400200A RID: 8202
		internal static BEServer.BESV_GAME_DATA battlEyeServerInitData = null;

		// Token: 0x0400200B RID: 8203
		internal static BEServer.BESV_BE_DATA battlEyeServerRunData = null;

		// Token: 0x0400200D RID: 8205
		private static uint _bytesSent;

		// Token: 0x0400200E RID: 8206
		private static uint _bytesReceived;

		// Token: 0x0400200F RID: 8207
		private static uint _packetsSent;

		// Token: 0x04002010 RID: 8208
		private static uint _packetsReceived;

		// Token: 0x04002011 RID: 8209
		private static SteamServerAdvertisement _currentServerAdvertisement;

		// Token: 0x04002012 RID: 8210
		private static ServerConnectParameters _currentServerConnectParameters;

		// Token: 0x04002013 RID: 8211
		private static CSteamID _server;

		// Token: 0x04002014 RID: 8212
		private static CSteamID _client;

		// Token: 0x04002015 RID: 8213
		private static CSteamID _user;

		// Token: 0x04002016 RID: 8214
		private static byte[] _clientHash;

		// Token: 0x04002017 RID: 8215
		private static string _clientName;

		// Token: 0x04002018 RID: 8216
		internal static List<SteamPlayer> _clients = new List<SteamPlayer>();

		// Token: 0x04002019 RID: 8217
		public static List<SteamPending> pending = new List<SteamPending>();

		// Token: 0x0400201A RID: 8218
		private static bool _isServer;

		// Token: 0x0400201B RID: 8219
		private static bool _isClient;

		// Token: 0x0400201C RID: 8220
		private static bool _isPro;

		// Token: 0x0400201D RID: 8221
		private static bool _isConnected;

		// Token: 0x0400201E RID: 8222
		internal static bool isWaitingForWorkshopResponse;

		/// <summary>
		/// After client submits EServerMessage.Authenticate we are waiting
		/// for the EClientMessage.Accepted response.
		/// </summary>
		// Token: 0x0400201F RID: 8223
		internal static bool isWaitingForAuthenticationResponse;

		/// <summary>
		/// Realtime that client sent EServerMessage.Authenticate request.
		/// </summary>
		// Token: 0x04002020 RID: 8224
		internal static double sentAuthenticationRequestTime;

		/// <summary>
		/// File IDs the client thinks the server advertised it was using, or null if UGC response was pending.
		/// Prevents the server from advertising a smaller or fake list of items.
		/// </summary>
		// Token: 0x04002021 RID: 8225
		private static List<PublishedFileId_t> waitingForExpectedWorkshopItems;

		/// <summary>
		/// Needed before loading level.
		/// </summary>
		// Token: 0x04002022 RID: 8226
		internal static ENPCHoliday authorityHoliday;

		// Token: 0x04002023 RID: 8227
		private static Provider.CachedWorkshopResponse currentServerWorkshopResponse;

		// Token: 0x04002024 RID: 8228
		private static List<ulong> _serverWorkshopFileIDs = new List<ulong>();

		// Token: 0x04002025 RID: 8229
		internal static List<Provider.ServerRequiredWorkshopFile> serverRequiredWorkshopFiles = new List<Provider.ServerRequiredWorkshopFile>();

		// Token: 0x04002026 RID: 8230
		public static bool isLoadingUGC;

		// Token: 0x04002027 RID: 8231
		public static bool isLoadingInventory;

		// Token: 0x04002028 RID: 8232
		private static int nextPlayerChannelId = 2;

		// Token: 0x04002029 RID: 8233
		public static ESteamConnectionFailureInfo _connectionFailureInfo;

		// Token: 0x0400202A RID: 8234
		internal static string _connectionFailureReason;

		// Token: 0x0400202B RID: 8235
		internal static uint _connectionFailureDuration;

		// Token: 0x0400202C RID: 8236
		private static List<SteamChannel> _receivers = new List<SteamChannel>();

		// Token: 0x0400202D RID: 8237
		internal static byte[] buffer = new byte[Block.BUFFER_SIZE];

		// Token: 0x0400202E RID: 8238
		internal static List<SDG.Framework.Modules.Module> critMods = new List<SDG.Framework.Modules.Module>();

		// Token: 0x0400202F RID: 8239
		private static StringBuilder modBuilder = new StringBuilder();

		// Token: 0x04002030 RID: 8240
		private static int nextBattlEyePlayerId = 1;

		/// <summary>
		/// Called when determining spawnpoint during player login.
		/// </summary>
		// Token: 0x04002031 RID: 8241
		public static Provider.LoginSpawningHandler onLoginSpawning;

		/// <summary>
		/// Is client waiting for response to ESteamPacket.CONNECT request?
		/// </summary>
		// Token: 0x04002032 RID: 8242
		internal static bool isWaitingForConnectResponse;

		/// <summary>
		/// Realtime that client sent ESteamPacket.CONNECT request.
		/// </summary>
		// Token: 0x04002033 RID: 8243
		private static float sentConnectRequestTime;

		/// <summary>
		/// Nelson 2023-08-09: adding because in some cases, namely workshop download and level loading,
		/// we can't properly handle client transport failures because these loading systems don't
		/// currently support cancelling partway through. (public issue #4036)
		/// </summary>
		// Token: 0x04002034 RID: 8244
		private static bool canCurrentlyHandleClientTransportFailure;

		// Token: 0x04002035 RID: 8245
		private static bool hasPendingClientTransportFailure;

		// Token: 0x04002036 RID: 8246
		private static string pendingClientTransportFailureMessage;

		// Token: 0x04002037 RID: 8247
		internal static readonly NetLength MAX_SKINS_LENGTH = new NetLength(127U);

		/// <summary>
		/// Manages client to server communication.
		/// </summary>
		// Token: 0x04002038 RID: 8248
		internal static IClientTransport clientTransport;

		/// <summary>
		/// Manages server to client communication.
		/// </summary>
		// Token: 0x04002039 RID: 8249
		private static IServerTransport serverTransport;

		// Token: 0x0400203B RID: 8251
		private static int countShutdownTimer = -1;

		// Token: 0x0400203C RID: 8252
		private static string shutdownMessage = string.Empty;

		// Token: 0x0400203D RID: 8253
		private static float lastTimerMessage;

		// Token: 0x0400203E RID: 8254
		internal static bool didServerShutdownTimerReachZero;

		/// <summary>
		/// Set on the server when initializing Steam API.
		/// Used to notify pending clients whether VAC is active.
		/// Set on clients after initial response is received.
		/// </summary>
		// Token: 0x0400203F RID: 8255
		internal static bool isVacActive;

		/// <summary>
		/// Set on the server when initializing BattlEye API.
		/// Used to notify pending clients whether BE is active.
		/// Set on clients after initial response is received.
		/// </summary>
		// Token: 0x04002040 RID: 8256
		internal static bool isBattlEyeActive;

		// Token: 0x04002041 RID: 8257
		private static bool hasSetIsBattlEyeActive;

		// Token: 0x04002042 RID: 8258
		private static bool isServerConnectedToSteam;

		// Token: 0x04002043 RID: 8259
		internal static BuiltinAutoShutdown autoShutdownManager = null;

		// Token: 0x04002044 RID: 8260
		private static IDedicatedWorkshopUpdateMonitor dswUpdateMonitor = null;

		// Token: 0x04002045 RID: 8261
		private static bool isDedicatedUGCInstalled;

		/// <summary>
		/// Was not able to find documentation for this unfortunately,
		/// but it seems the max length is 127 characters as of 2022-09-12.
		/// </summary>
		// Token: 0x04002046 RID: 8262
		private const int STEAM_KEYVALUE_MAX_VALUE_LENGTH = 127;

		// Token: 0x04002047 RID: 8263
		[Obsolete]
		public static Provider.ServerWritingPacketHandler onServerWritingPacket;

		// Token: 0x04002048 RID: 8264
		internal static List<Provider.WorkshopRequestLog> workshopRequests = new List<Provider.WorkshopRequestLog>();

		// Token: 0x04002049 RID: 8265
		internal static List<Provider.CachedWorkshopResponse> cachedWorkshopResponses = new List<Provider.CachedWorkshopResponse>();

		// Token: 0x0400204A RID: 8266
		private static List<CSteamID> netIgnoredSteamIDs = new List<CSteamID>();

		/// <summary>
		/// Private to prevent plugins from changing the value.
		/// </summary>
		// Token: 0x0400204B RID: 8267
		private static CommandLineFlag _constNetEvents = new CommandLineFlag(false, "-ConstNetEvents");

		// Token: 0x0400204C RID: 8268
		[Obsolete]
		public static Provider.ServerReadingPacketHandler onServerReadingPacket;

		// Token: 0x0400204D RID: 8269
		private List<SteamPlayer> clientsWithBadConnecion = new List<SteamPlayer>();

		// Token: 0x0400204E RID: 8270
		public static Provider.ServerConnected onServerConnected;

		// Token: 0x0400204F RID: 8271
		public static Provider.ServerDisconnected onServerDisconnected;

		// Token: 0x04002050 RID: 8272
		public static Provider.ServerHosted onServerHosted;

		// Token: 0x04002051 RID: 8273
		public static Provider.ServerShutdown onServerShutdown;

		// Token: 0x04002052 RID: 8274
		private static Callback<P2PSessionConnectFail_t> p2pSessionConnectFail;

		// Token: 0x04002053 RID: 8275
		[Obsolete("onCheckValidWithExplanation takes priority if bound")]
		public static Provider.CheckValid onCheckValid;

		// Token: 0x04002054 RID: 8276
		public static Provider.CheckValidWithExplanation onCheckValidWithExplanation;

		// Token: 0x04002055 RID: 8277
		[Obsolete]
		public static Provider.CheckBanStatusHandler onCheckBanStatus;

		// Token: 0x04002056 RID: 8278
		public static Provider.CheckBanStatusWithHWIDHandler onCheckBanStatusWithHWID;

		// Token: 0x04002057 RID: 8279
		[Obsolete("V2 provides list of HWIDs to ban")]
		public static Provider.RequestBanPlayerHandler onBanPlayerRequested;

		// Token: 0x04002058 RID: 8280
		public static Provider.RequestBanPlayerHandlerV2 onBanPlayerRequestedV2;

		// Token: 0x04002059 RID: 8281
		public static Provider.RequestUnbanPlayerHandler onUnbanPlayerRequested;

		// Token: 0x0400205A RID: 8282
		private static Callback<ValidateAuthTicketResponse_t> validateAuthTicketResponse;

		// Token: 0x0400205B RID: 8283
		private static Callback<GSClientGroupStatus_t> clientGroupStatus;

		/// <summary>
		/// Allows hosting providers to limit the configurable max players value from the command-line.
		/// </summary>
		// Token: 0x0400205C RID: 8284
		private static CommandLineInt clMaxPlayersLimit = new CommandLineInt("-MaxPlayersLimit");

		// Token: 0x0400205D RID: 8285
		private static byte _maxPlayers;

		// Token: 0x0400205E RID: 8286
		public static byte queueSize;

		// Token: 0x0400205F RID: 8287
		internal static byte _queuePosition;

		// Token: 0x04002060 RID: 8288
		public static Provider.QueuePositionUpdated onQueuePositionUpdated;

		// Token: 0x04002061 RID: 8289
		private static string _serverName;

		/// <summary>
		/// Deprecated-ish IPv4 to bind listen socket to. Set by bind command.
		/// </summary>
		// Token: 0x04002062 RID: 8290
		public static uint ip;

		/// <summary>
		/// Local address to bind listen socket to. Set by bind command.
		/// </summary>
		// Token: 0x04002063 RID: 8291
		public static string bindAddress;

		/// <summary>
		/// Steam query port.
		/// </summary>
		// Token: 0x04002064 RID: 8292
		public static ushort port;

		// Token: 0x04002065 RID: 8293
		internal static byte[] _serverPasswordHash;

		// Token: 0x04002066 RID: 8294
		private static string _serverPassword;

		// Token: 0x04002067 RID: 8295
		public static string map;

		// Token: 0x04002068 RID: 8296
		public static bool isPvP;

		// Token: 0x04002069 RID: 8297
		public static bool isWhitelisted;

		// Token: 0x0400206A RID: 8298
		public static bool hideAdmins;

		// Token: 0x0400206B RID: 8299
		public static bool hasCheats;

		// Token: 0x0400206C RID: 8300
		public static bool filterName;

		// Token: 0x0400206D RID: 8301
		public static EGameMode mode;

		// Token: 0x0400206E RID: 8302
		public static bool isGold;

		// Token: 0x0400206F RID: 8303
		public static GameMode gameMode;

		// Token: 0x04002070 RID: 8304
		public static ECameraMode cameraMode;

		// Token: 0x04002071 RID: 8305
		private static StatusData _statusData;

		// Token: 0x04002072 RID: 8306
		private static PreferenceData _preferenceData;

		// Token: 0x04002073 RID: 8307
		private static ConfigData _configData;

		// Token: 0x04002074 RID: 8308
		internal static ModeConfigData _modeConfigData;

		/// <summary>
		/// Number of transport connection failures on this frame.
		/// </summary>
		// Token: 0x04002076 RID: 8310
		private int clientsKickedForTransportConnectionFailureCount;

		// Token: 0x04002077 RID: 8311
		private static uint STEAM_FAVORITE_FLAG_FAVORITE = 1U;

		// Token: 0x04002078 RID: 8312
		internal static uint STEAM_FAVORITE_FLAG_HISTORY = 2U;

		// Token: 0x04002079 RID: 8313
		private static List<Provider.CachedFavorite> cachedFavorites = new List<Provider.CachedFavorite>();

		// Token: 0x0400207A RID: 8314
		public static Provider.ClientConnected onClientConnected;

		// Token: 0x0400207B RID: 8315
		public static Provider.ClientDisconnected onClientDisconnected;

		// Token: 0x0400207C RID: 8316
		public static Provider.EnemyConnected onEnemyConnected;

		// Token: 0x0400207D RID: 8317
		public static Provider.EnemyDisconnected onEnemyDisconnected;

		// Token: 0x0400207E RID: 8318
		private static Callback<PersonaStateChange_t> personaStateChange;

		// Token: 0x0400207F RID: 8319
		private static Callback<GameServerChangeRequested_t> gameServerChangeRequested;

		// Token: 0x04002080 RID: 8320
		private static Callback<GameRichPresenceJoinRequested_t> gameRichPresenceJoinRequested;

		// Token: 0x04002081 RID: 8321
		private static HAuthTicket ticketHandle = HAuthTicket.Invalid;

		// Token: 0x04002082 RID: 8322
		private static float lastPingRequestTime;

		// Token: 0x04002083 RID: 8323
		private static float lastQueueNotificationTime;

		// Token: 0x04002085 RID: 8325
		internal static float timeLastPingRequestWasSentToServer;

		// Token: 0x04002086 RID: 8326
		public static readonly float EPSILON = 0.01f;

		// Token: 0x04002087 RID: 8327
		public static readonly float UPDATE_TIME = 0.08f;

		// Token: 0x04002088 RID: 8328
		public static readonly float UPDATE_DELAY = 0.1f;

		// Token: 0x04002089 RID: 8329
		public static readonly float UPDATE_DISTANCE = 0.01f;

		// Token: 0x0400208A RID: 8330
		public static readonly uint UPDATES = 1U;

		// Token: 0x0400208B RID: 8331
		public static readonly float LERP = 3f;

		// Token: 0x0400208C RID: 8332
		internal const float INTERP_SPEED = 10f;

		// Token: 0x0400208D RID: 8333
		private static float[] pings;

		// Token: 0x0400208E RID: 8334
		private static float _ping;

		// Token: 0x0400208F RID: 8335
		private static Provider steam;

		// Token: 0x04002091 RID: 8337
		private static bool _isInitialized;

		// Token: 0x04002092 RID: 8338
		private static uint timeOffset;

		// Token: 0x04002093 RID: 8339
		private static uint _time;

		// Token: 0x04002094 RID: 8340
		private static uint initialBackendRealtimeSeconds;

		// Token: 0x04002095 RID: 8341
		private static float initialLocalRealtime;

		/// <summary>
		/// Current UTC as reported by backend servers.
		/// Used by holiday events to keep timing somewhat synced between players. 
		/// </summary>
		// Token: 0x04002096 RID: 8342
		private static DateTime unixEpochDateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

		/// <summary>
		/// Invoked after backend realtime becomes available.
		/// </summary>
		// Token: 0x04002097 RID: 8343
		public static Provider.BackendRealtimeAvailableHandler onBackendRealtimeAvailable;

		// Token: 0x04002098 RID: 8344
		private static SteamAPIWarningMessageHook_t apiWarningMessageHook;

		// Token: 0x04002099 RID: 8345
		private static int debugUpdates;

		// Token: 0x0400209A RID: 8346
		public static int debugUPS;

		// Token: 0x0400209B RID: 8347
		private static float debugLastUpdate;

		// Token: 0x0400209C RID: 8348
		private static int debugTicks;

		// Token: 0x0400209D RID: 8349
		public static int debugTPS;

		// Token: 0x0400209E RID: 8350
		private static float debugLastTick;

		// Token: 0x0400209F RID: 8351
		private static Dictionary<string, Texture2D> downloadedIconCache = new Dictionary<string, Texture2D>();

		// Token: 0x040020A0 RID: 8352
		private static Dictionary<string, Provider.PendingIconRequest> pendingCachableIconRequests = new Dictionary<string, Provider.PendingIconRequest>();

		// Token: 0x040020A1 RID: 8353
		internal static CommandLineFlag allowWebRequests = new CommandLineFlag(true, "-NoWebRequests");

		// Token: 0x040020A3 RID: 8355
		private static bool wasQuitGameCalled;

		/// <summary>
		/// A couple of players have reported the PRO_DESYNC kick because their client thinks they own the gold upgrade,
		/// but the Steam backend thinks otherwise. This option is a bit of a hack to work around the problem for them.
		/// </summary>
		// Token: 0x040020A4 RID: 8356
		private static CommandLineFlag shouldCheckForGoldUpgrade = new CommandLineFlag(true, "-NoGoldUpgrade");

		// Token: 0x0200098D RID: 2445
		// (Invoke) Token: 0x06004AA3 RID: 19107
		public delegate void BattlEyeKickCallback(SteamPlayer client, string reason);

		// Token: 0x0200098E RID: 2446
		internal struct ServerRequiredWorkshopFile
		{
			// Token: 0x040032D8 RID: 13016
			public ulong fileId;

			// Token: 0x040032D9 RID: 13017
			public DateTime timestamp;
		}

		// Token: 0x0200098F RID: 2447
		// (Invoke) Token: 0x06004AA7 RID: 19111
		public delegate void LoginSpawningHandler(SteamPlayerID playerID, ref Vector3 point, ref float yaw, ref EPlayerStance initialStance, ref bool needsNewSpawnpoint);

		// Token: 0x02000990 RID: 2448
		// (Invoke) Token: 0x06004AAB RID: 19115
		public delegate void CommenceShutdownHandler();

		// Token: 0x02000991 RID: 2449
		// (Invoke) Token: 0x06004AAF RID: 19119
		[Obsolete]
		public delegate void ServerWritingPacketHandler(CSteamID remoteSteamId, ESteamPacket type, byte[] payload, int size, int channel);

		/// <summary>
		/// Workshop info is requested prior to authenticating so that it can be downloaded before joining,
		/// but cheat devs are abusing this to spam the server with workshop requests. This class keeps
		/// track of who and when requested that information.
		/// </summary>
		// Token: 0x02000992 RID: 2450
		internal struct WorkshopRequestLog
		{
			/// <summary>
			/// Hash code of remote connection.
			/// </summary>
			// Token: 0x040032DA RID: 13018
			public int sender;

			// Token: 0x040032DB RID: 13019
			public float realTime;
		}

		/// <summary>
		/// The server ignores workshop info requests if it's been less than 30 seconds,
		/// so we cache that info for 1 minute in-case we try to connect again right away.
		/// </summary>
		// Token: 0x02000993 RID: 2451
		internal class CachedWorkshopResponse
		{
			// Token: 0x06004AB2 RID: 19122 RVA: 0x001BC978 File Offset: 0x001BAB78
			internal bool FindRequiredFile(ulong fileId, out Provider.ServerRequiredWorkshopFile details)
			{
				foreach (Provider.ServerRequiredWorkshopFile serverRequiredWorkshopFile in this.requiredFiles)
				{
					if (serverRequiredWorkshopFile.fileId == fileId)
					{
						details = serverRequiredWorkshopFile;
						return true;
					}
				}
				details = default(Provider.ServerRequiredWorkshopFile);
				return false;
			}

			/// <summary>
			/// This information is needed before the level is loaded.
			/// </summary>
			// Token: 0x040032DC RID: 13020
			public ENPCHoliday holiday;

			/// <summary>
			/// Advertised server name. e.g., "Nelson's Unturned Server"
			/// </summary>
			// Token: 0x040032DD RID: 13021
			public string serverName;

			/// <summary>
			/// Name of map to load.
			/// </summary>
			// Token: 0x040032DE RID: 13022
			public string levelName;

			// Token: 0x040032DF RID: 13023
			public bool isPvP;

			// Token: 0x040032E0 RID: 13024
			public bool allowAdminCheatCodes;

			// Token: 0x040032E1 RID: 13025
			public bool isVACSecure;

			// Token: 0x040032E2 RID: 13026
			public bool isBattlEyeSecure;

			// Token: 0x040032E3 RID: 13027
			public bool isGold;

			/// <summary>
			/// Legacy difficulty mode that should be removed eventually.
			/// </summary>
			// Token: 0x040032E4 RID: 13028
			public EGameMode gameMode;

			/// <summary>
			/// Perspective settings.
			/// </summary>
			// Token: 0x040032E5 RID: 13029
			public ECameraMode cameraMode;

			// Token: 0x040032E6 RID: 13030
			public byte maxPlayers;

			// Token: 0x040032E7 RID: 13031
			public CSteamID server;

			/// <summary>
			/// Server's IP from when we originally received response.
			/// Used to test download restrictions.
			/// </summary>
			// Token: 0x040032E8 RID: 13032
			public uint ip;

			// Token: 0x040032E9 RID: 13033
			public List<Provider.ServerRequiredWorkshopFile> requiredFiles = new List<Provider.ServerRequiredWorkshopFile>();

			/// <summary>
			/// Last realtime this cache was updated.
			/// </summary>
			// Token: 0x040032EA RID: 13034
			public float realTime;
		}

		// Token: 0x02000994 RID: 2452
		// (Invoke) Token: 0x06004AB5 RID: 19125
		public delegate void ServerReadingPacketHandler(CSteamID remoteSteamId, byte[] payload, int offset, int size, int channel);

		// Token: 0x02000995 RID: 2453
		// (Invoke) Token: 0x06004AB9 RID: 19129
		public delegate void ServerConnected(CSteamID steamID);

		// Token: 0x02000996 RID: 2454
		// (Invoke) Token: 0x06004ABD RID: 19133
		public delegate void ServerDisconnected(CSteamID steamID);

		// Token: 0x02000997 RID: 2455
		// (Invoke) Token: 0x06004AC1 RID: 19137
		public delegate void ServerHosted();

		// Token: 0x02000998 RID: 2456
		// (Invoke) Token: 0x06004AC5 RID: 19141
		public delegate void ServerShutdown();

		// Token: 0x02000999 RID: 2457
		// (Invoke) Token: 0x06004AC9 RID: 19145
		[Obsolete]
		public delegate void CheckValid(ValidateAuthTicketResponse_t callback, ref bool isValid);

		// Token: 0x0200099A RID: 2458
		// (Invoke) Token: 0x06004ACD RID: 19149
		public delegate void CheckValidWithExplanation(ValidateAuthTicketResponse_t callback, ref bool isValid, ref string explanation);

		// Token: 0x0200099B RID: 2459
		// (Invoke) Token: 0x06004AD1 RID: 19153
		public delegate void CheckBanStatusHandler(CSteamID steamID, uint remoteIP, ref bool isBanned, ref string banReason, ref uint banRemainingDuration);

		// Token: 0x0200099C RID: 2460
		// (Invoke) Token: 0x06004AD5 RID: 19157
		public delegate void CheckBanStatusWithHWIDHandler(SteamPlayerID playerID, uint remoteIP, ref bool isBanned, ref string banReason, ref uint banRemainingDuration);

		// Token: 0x0200099D RID: 2461
		// (Invoke) Token: 0x06004AD9 RID: 19161
		public delegate void RequestBanPlayerHandler(CSteamID instigator, CSteamID playerToBan, uint ipToBan, ref string reason, ref uint duration, ref bool shouldVanillaBan);

		// Token: 0x0200099E RID: 2462
		// (Invoke) Token: 0x06004ADD RID: 19165
		public delegate void RequestBanPlayerHandlerV2(CSteamID instigator, CSteamID playerToBan, uint ipToBan, IEnumerable<byte[]> hwidsToBan, ref string reason, ref uint duration, ref bool shouldVanillaBan);

		// Token: 0x0200099F RID: 2463
		// (Invoke) Token: 0x06004AE1 RID: 19169
		public delegate void RequestUnbanPlayerHandler(CSteamID instigator, CSteamID playerToUnban, ref bool shouldVanillaUnban);

		// Token: 0x020009A0 RID: 2464
		// (Invoke) Token: 0x06004AE5 RID: 19173
		public delegate void QueuePositionUpdated();

		// Token: 0x020009A1 RID: 2465
		// (Invoke) Token: 0x06004AE9 RID: 19177
		public delegate void RejectingPlayerCallback(CSteamID steamID, ESteamRejection rejection, string explanation);

		// Token: 0x020009A2 RID: 2466
		private class CachedFavorite
		{
			// Token: 0x06004AEC RID: 19180 RVA: 0x001BC9F7 File Offset: 0x001BABF7
			public bool matchesServer(uint ip, ushort queryPort)
			{
				return this.ip == ip && this.queryPort == queryPort;
			}

			// Token: 0x040032EB RID: 13035
			public uint ip;

			// Token: 0x040032EC RID: 13036
			public ushort queryPort;

			// Token: 0x040032ED RID: 13037
			public bool isFavorited;
		}

		// Token: 0x020009A3 RID: 2467
		// (Invoke) Token: 0x06004AEF RID: 19183
		public delegate void ClientConnected();

		// Token: 0x020009A4 RID: 2468
		// (Invoke) Token: 0x06004AF3 RID: 19187
		public delegate void ClientDisconnected();

		// Token: 0x020009A5 RID: 2469
		// (Invoke) Token: 0x06004AF7 RID: 19191
		public delegate void EnemyConnected(SteamPlayer player);

		// Token: 0x020009A6 RID: 2470
		// (Invoke) Token: 0x06004AFB RID: 19195
		public delegate void EnemyDisconnected(SteamPlayer player);

		// Token: 0x020009A7 RID: 2471
		// (Invoke) Token: 0x06004AFF RID: 19199
		public delegate void BackendRealtimeAvailableHandler();

		// Token: 0x020009A8 RID: 2472
		// (Invoke) Token: 0x06004B03 RID: 19203
		public delegate void IconQueryCallback(Texture2D icon, bool responsibleForDestroy);

		// Token: 0x020009A9 RID: 2473
		public struct IconQueryParams
		{
			// Token: 0x06004B06 RID: 19206 RVA: 0x001BCA15 File Offset: 0x001BAC15
			public IconQueryParams(string url, Provider.IconQueryCallback callback, bool shouldCache = true)
			{
				this.url = url;
				this.callback = callback;
				this.shouldCache = shouldCache;
			}

			// Token: 0x040032EE RID: 13038
			public string url;

			// Token: 0x040032EF RID: 13039
			public Provider.IconQueryCallback callback;

			// Token: 0x040032F0 RID: 13040
			public bool shouldCache;
		}

		// Token: 0x020009AA RID: 2474
		private class PendingIconRequest
		{
			// Token: 0x040032F1 RID: 13041
			public string url;

			// Token: 0x040032F2 RID: 13042
			public Provider.IconQueryCallback callback;

			// Token: 0x040032F3 RID: 13043
			public bool shouldCache;
		}
	}
}
