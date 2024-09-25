#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include "test_audio.h" // The Wav file
#include "../features/preferences_module.h"
// Audio audio;

static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number
unsigned const char *wav_data = 0;
uint32_t wav_data_index = 0; // index offset into "wav_data" for current  data t send to I2S

struct WavHeader_Struct
{
    //   RIFF Section
    char RIFFSectionID[4]; // Letters "RIFF"
    uint32_t Size;         // Size of entire file less 8
    char RiffFormat[4];    // Letters "WAVE"

    //   Format Section
    char FormatSectionID[4]; // letters "fmt"
    uint32_t FormatSize;     // Size of format section less 8
    uint16_t FormatID;       // 1=uncompressed PCM
    uint16_t NumChannels;    // 1=mono,2=stereo
    uint32_t SampleRate;     // 44100, 16000, 8000 etc.
    uint32_t ByteRate;       // =SampleRate * Channels * (BitsPerSample/8)
    uint16_t BlockAlign;     // =Channels * (BitsPerSample/8)
    uint16_t BitsPerSample;  // 8,16,24 or 32

    // Data Section
    char DataSectionID[4]; // The letters "data"
    uint32_t DataSize;     // Size of the data that follows
} WavHeader;

static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
    .dma_buf_count = 8,                       // 8 buffers
    .dma_buf_len = 1024,                      // 1K per buffer, so 8K of buffer space
    .use_apll = 0,
    .tx_desc_auto_clear = true,
    .fixed_mclk = -1};

void PrintData(const char *Data, uint8_t NumBytes)
{
    for (uint8_t i = 0; i < NumBytes; i++)
        Serial.print(Data[i]);
    Serial.println();
}

bool ValidWavData(WavHeader_Struct *Wav)
{

    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        Serial.print("Invlaid data - Not RIFF format");
        return false;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        Serial.print("Invlaid data - Not Wave file");
        return false;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        Serial.print("Invlaid data - No format section found");
        return false;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        Serial.print("Invlaid data - data section not found");
        return false;
    }
    if (Wav->FormatID != 1)
    {
        Serial.print("Invlaid data - format Id must be 1");
        return false;
    }
    if (Wav->FormatSize != 16)
    {
        Serial.print("Invlaid data - format section size must be 16.");
        return false;
    }
    if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
    {
        Serial.print("Invlaid data - only mono or stereo permitted.");
        return false;
    }
    if (Wav->SampleRate > 48000)
    {
        Serial.print("Invlaid data - Sample rate cannot be greater than 48000");
        return false;
    }
    if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
    {
        Serial.print("Invlaid data - Only 8 or 16 bits per sample permitted.");
        return false;
    }
    return true;
}

void DumpWAVHeader(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        Serial.print("Not a RIFF format file - ");
        PrintData(Wav->RIFFSectionID, 4);
        return;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        Serial.print("Not a WAVE file - ");
        PrintData(Wav->RiffFormat, 4);
        return;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        Serial.print("fmt ID not present - ");
        PrintData(Wav->FormatSectionID, 3);
        return;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        Serial.print("data ID not present - ");
        PrintData(Wav->DataSectionID, 4);
        return;
    }
    // All looks good, dump the data
    Serial.print("Total size :");
    Serial.println(Wav->Size);
    Serial.print("Format section size :");
    Serial.println(Wav->FormatSize);
    Serial.print("Wave format :");
    Serial.println(Wav->FormatID);
    Serial.print("Channels :");
    Serial.println(Wav->NumChannels);
    Serial.print("Sample Rate :");
    Serial.println(Wav->SampleRate);
    Serial.print("Byte Rate :");
    Serial.println(Wav->ByteRate);
    Serial.print("Block Align :");
    Serial.println(Wav->BlockAlign);
    Serial.print("Bits Per Sample :");
    Serial.println(Wav->BitsPerSample);
    Serial.print("Data Size :");
    Serial.println(Wav->DataSize);
}

void play_audio_from_header_file()
{

    const unsigned char *WavFile = audio_data_from_h_file;
    memcpy(&WavHeader, WavFile, 44); // Copy the header part of the wav data into our structure
    DumpWAVHeader(&WavHeader);       // Dump the header data to serial, optional!
    if (ValidWavData(&WavHeader))
    {
        i2s_set_sample_rates(i2s_num, WavHeader.SampleRate); // set sample rate
        wav_data = WavFile + 44;
        wav_data_index = 0;
    }
}

void setup_audio()
{
    i2s_pin_config_t pin_config = {
        .bck_io_num = preferences_i2s_bck_pin(),    // The bit clock connectiom, goes to pin 27 of ESP32
        .ws_io_num = preferences_i2s_ws_pin(),      // Word select, also known as word select or left right clock
        .data_out_num = preferences_i2s_data_pin(), // Data out from the ESP32, connect to DIN on 38357A
        .data_in_num = I2S_PIN_NO_CHANGE            // we are not interested in I2S data into the ESP32
    };

    i2s_driver_install(i2s_num, &i2s_config, 0, NULL); // ESP32 will allocated resources to run I2S
    i2s_set_pin(i2s_num, &pin_config);                 // Tell it the pins you will be using
}

void play_audio_from_spiffs(const char *filename, uint8_t action)
{

    Serial.println("Trying to find audio file: ");
    Serial.print(filename);

    // Find a media file and update status to 1
    for (uint8_t file_ind = 0; file_ind < board_settings.media_files_amount; ++file_ind)
    {
        MediaFile file = board_settings.media_files[file_ind];

        if (strcmp(file.file_name, filename) == 0)
        {
            // If we should stop sound just update the status, we shouldn't check wav
            if (action != 1)
            {
                board_settings.media_files[file_ind].status = action;
            }

            if (board_settings.media_files[file_ind].file.available())
            {

                uint8_t wav_header[44];
                file.file.seek(0);
                board_settings.media_files[file_ind].file.read(wav_header, 44);
                memcpy(&WavHeader, wav_header, 44); // Copy the header part of the wav data into our structure
                DumpWAVHeader(&WavHeader);          // Dump the header data to serial, optional!
                if (ValidWavData(&WavHeader))
                {
                    i2s_set_sample_rates(i2s_num, WavHeader.SampleRate); // set sample rate
                                                                         // file.seek(44);
                    board_settings.media_files[file_ind].status = action;

                    board_settings.media_files[file_ind].wav_data_size = WavHeader.DataSize;
                    board_settings.media_files[file_ind].channels = WavHeader.NumChannels;
                    board_settings.media_files[file_ind].wav_data_index = 0;
                }
            }
        }
    }
}

void loop_audio()
{

    // Iterate over madia files with status 1 and play them

    const uint8_t bytes_to_read = 4; // bytes to read in mono

    uint8_t pcm_data[2 * bytes_to_read] = {0};
    bool is_should_play = false;

    for (uint8_t file_ind = 0; file_ind < board_settings.media_files_amount; ++file_ind)
    {

        MediaFile file = board_settings.media_files[file_ind];
        if (file.status == 1)
        {

            uint8_t cur_pcm_data[2 * bytes_to_read] = {0};

            if (file.channels == 1)
            {
                uint8_t cur_mono_pcm_data[bytes_to_read] = {0};

                file.file.read(cur_mono_pcm_data, bytes_to_read);

                for (uint8_t ind = 0; ind < bytes_to_read; ind += 2)
                {
                    cur_pcm_data[2 * ind] = cur_mono_pcm_data[ind];
                    cur_pcm_data[2 * ind + 1] = cur_mono_pcm_data[ind + 1];

                    cur_pcm_data[2 * ind + 2] = cur_mono_pcm_data[ind];
                    cur_pcm_data[2 * ind + 3] = cur_mono_pcm_data[ind + 1];
                }
                board_settings.media_files[file_ind].wav_data_index += bytes_to_read;
            }
            else
            {

                file.file.read(cur_pcm_data, 2 * bytes_to_read); // in stereo we read x2 bytes (2 channels)
                board_settings.media_files[file_ind].wav_data_index += 2 * bytes_to_read;
            }

            for (uint8_t ind = 0; ind < 2 * bytes_to_read; ind += 2)
            {
                *((int16_t *)(pcm_data + ind)) += *((int16_t *)(cur_pcm_data + ind));
                *((int16_t *)(pcm_data + ind)) /= 2;
            }

            if (board_settings.media_files[file_ind].wav_data_index >= file.wav_data_size) // If we gone past end of data reset back to beginning
            {
                Serial.print("Restart audio");

                board_settings.media_files[file_ind].wav_data_index = 0;
                file.file.seek(44);
            }

            is_should_play = true;
        }
    }
    if (is_should_play)
    {
        size_t BytesWritten; // Returned by the I2S write routine, we are not interested in it
        i2s_write(i2s_num, pcm_data, 8, &BytesWritten, portMAX_DELAY);
    }
}

#endif