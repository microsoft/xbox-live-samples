#if NETFX_CORE
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.Xbox.Services;
using Windows.Gaming.XboxLive.Storage;
using Windows.Storage.Streams;

// Used to return the results of a LoadData call.
public struct LoadDataResult
{
    public LoadDataResult(GameSaveErrorStatus statusValue, int dataValue)
    {
        Status = statusValue;
        Data = dataValue;
    }

    public GameSaveErrorStatus Status;
    public int Data;
}

// This is a simple example class demonstrating how to call GameSave APIs. It is not intended to implement
// a fully-featured or robust save system.
public class GameSaveManager
{
    const string c_saveContainerName = "GameSaveContainer";
    const string c_saveContainerDisplayName = "Game Save";
    const string c_saveBlobName = "data";

    XboxLiveContext m_context;
    GameSaveProvider m_saveProvider;

    public async Task<GameSaveErrorStatus> Initialize(XboxLiveContext context)
    {
        if (context == null)
        {
            throw new ArgumentNullException("context");
        }

        // Getting a GameSaveProvider requires the Windows user object. It will automatically get the correct
        // provider for the current Xbox Live user.
        var users = await Windows.System.User.FindAllAsync();

        if (users.Count > 0)
        {
            GameSaveProviderGetResult result = await GameSaveProvider.GetForUserAsync(
                users[0], context.AppConfig.ServiceConfigurationId);
            if (result.Status == GameSaveErrorStatus.Ok)
            {
                m_context = context;
                m_saveProvider = result.Value;
            }

            return result.Status;
        }
        else
        {
            throw new Exception("No Windows users found when creating save provider.");
        }
    }

    public void Reset()
    {
        m_context = null;
        m_saveProvider = null;
    }

    public async Task<GameSaveErrorStatus> SaveData(int data)
    {
        if(m_saveProvider == null)
        {
            throw new InvalidOperationException("The save system is not initialized.");
        }

        GameSaveContainer container = m_saveProvider.CreateContainer(c_saveContainerName);
        
        // To store a value in the container, it needs to be written into a buffer, then stored with
        // a blob name in a Dictionary.
        DataWriter writer = new DataWriter();
        writer.WriteInt32(data);
        IBuffer dataBuffer = writer.DetachBuffer();

        var updates = new Dictionary<string, IBuffer>();
        updates.Add(c_saveBlobName, dataBuffer);

        GameSaveOperationResult result = await container.SubmitUpdatesAsync(updates, null, c_saveContainerDisplayName);
        return result.Status;
    }

    public async Task<LoadDataResult> LoadData()
    {
        if (m_saveProvider == null)
        {
            throw new InvalidOperationException("The save system is not initialized.");
        }

        GameSaveContainer container = m_saveProvider.CreateContainer(c_saveContainerName);

        string[] blobsToRead = new string[] { c_saveBlobName };

        // GetAsync allocates a new Dictionary to hold the retrieved data. You can also use ReadAsync
        // to provide your own preallocated Dictionary.
        GameSaveBlobGetResult result = await container.GetAsync(blobsToRead);

        int loadedData = 0;

        if(result.Status == GameSaveErrorStatus.Ok)
        {
            IBuffer loadedBuffer;
            result.Value.TryGetValue(c_saveBlobName, out loadedBuffer);

            if(loadedBuffer == null)
            {
                throw new Exception(String.Format("Didn't find expected blob \"{0}\" in the loaded data.", c_saveBlobName));
            }

            DataReader reader = DataReader.FromBuffer(loadedBuffer);
            loadedData = reader.ReadInt32();
        }

        return new LoadDataResult(result.Status, loadedData);
    }

    public async Task<GameSaveContainerInfoGetResult> GetContainerInfo()
    {
        if (m_saveProvider == null)
        {
            throw new InvalidOperationException("The save system is not initialized.");
        }

        GameSaveContainerInfoQuery query = m_saveProvider.CreateContainerInfoQuery();
        return await query.GetContainerInfoAsync();
    }

    public async Task<GameSaveErrorStatus> DeleteContainer()
    {
        if (m_saveProvider == null)
        {
            throw new InvalidOperationException("The save system is not initialized.");
        }

        GameSaveOperationResult result = await m_saveProvider.DeleteContainerAsync(c_saveContainerName);
        return result.Status;
    }
}

#endif