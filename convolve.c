
//--Include files--//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


//--Declariation of instance vars--//
char chunkID[5];
char format[5];
char subChunk1ID[5];
char subChunk2ID[5];

int sample_Rate;
int byte_Rate;
int chunkSize;
int channel_Size;
int subChunk1Size;
int subChunk2Size;
int number_SamplesIR;


short audio_Format;
short number_Channels;
short block_Align;
short bits_Per_Sample;

// Arrays for input, output, and IR files
short* data;
short* dataIR;

//-------------------------------------//

int loadWave(char* filename)
{
    FILE* inFile = fopen(filename, "rb");
    if (inFile != NULL)
    {
        printf("\nNow Reading Dry Sound file: %s\n", filename);
        fread(chunkID, 1, 4, inFile);
        fread(&chunkSize, 1, 4, inFile);
        fread(format, 1, 4, inFile);

        fread(subChunk1ID, 1, 4, inFile);
        fread(&subChunk1Size, 1, 4, inFile);
        fread(&audio_Format, 1, 2, inFile);
        fread(&number_Channels, 1, 2, inFile);
        fread(&sample_Rate, 1, 4, inFile);
        fread(&byte_Rate, 1, 4, inFile);
        fread(&block_Align, 1, 2, inFile);
        fread(&bits_Per_Sample, 1, 2, inFile);

        if (subChunk1Size == 18)
        {
            short empty;
            fread(&empty, 1, 2, inFile);
        }

        fread(subChunk2ID, 1, 4, inFile);
        fread(&subChunk2Size, 1, 4, inFile);

        int bytes_Per_Sample = bits_Per_Sample / 8;
        int number_Samples = subChunk2Size / bytes_Per_Sample;
        data = (short*)malloc(sizeof(short) * number_Samples);



        int i = 0;
        short sample = 0;
        while (fread(&sample, 1, bytes_Per_Sample, inFile) == bytes_Per_Sample)
        {
            data[i++] = sample;
            sample = 0;
        }

        fclose(inFile);
        printf("Now Closing Dry Sound file: %s.\n", filename);
    }
    else
    {
        printf("Couldn't open: %s\n", filename);
        return 0;
    }
    return 1;
}


int saveWave(char* filename)
{
    FILE* outFile = fopen(filename, "wb");
    if (outFile != NULL)
    {
        printf("\nNow Writing Output file: %s\n", filename);

        fwrite(chunkID, 1, 4, outFile);
        fwrite(&chunkSize, 1, 4, outFile);
        fwrite(format, 1, 4, outFile);

        fwrite(subChunk1ID, 1, 4, outFile);
        fwrite(&subChunk1Size, 1, 4, outFile);
        fwrite(&audio_Format, 1, 2, outFile);
        fwrite(&number_Channels, 1, 2, outFile);
        fwrite(&sample_Rate, 1, 4, outFile);
        fwrite(&byte_Rate, 1, 4, outFile);
        fwrite(&block_Align, 1, 2, outFile);
        fwrite(&bits_Per_Sample, 1, 2, outFile);

        if (subChunk1Size == 18)
        {
            short empty = 0;
            fwrite(&empty, 1, 2, outFile);
        }

        fwrite(subChunk2ID, 1, 4, outFile);
        fwrite(&subChunk2Size, 1, 4, outFile);

        int bytes_Per_Sample = bits_Per_Sample / 8;
        int sample_Count = subChunk2Size / bytes_Per_Sample;

        printf("Size of Sample Count: %d\n", sample_Count);

        int IRSize = number_SamplesIR;

        float* newData = (float*)malloc(sizeof(float) * (sample_Count + IRSize - 1));
        float max_Sample = -1;
        float MAX_VAL = 32767.f;

				//Convolution Start//
				// Outerloop: process all values in turn //
        for (int i = 0; i < IRSize; i++)
        {
						//processs x[n] with each sampe of h[]
            for (int j = 0; j < sample_Count; j++)
            {
                newData[i + j] += ((float)data[j] / MAX_VAL) * ((float)dataIR[i] / MAX_VAL);
            }
            if (i == 0)
            {
                max_Sample = newData[0];
            }
            else if (newData[i] > max_Sample)
            {
                max_Sample = newData[i];
            }
        }

        for (int i = 0; i < sample_Count + IRSize - 1; ++i)
        {
            newData[i] = (newData[i] / max_Sample);
            short sample = (short)(newData[i] * MAX_VAL);
            fwrite(&sample, 1, bytes_Per_Sample, outFile);
        }

        //clean up
        free(newData);
        fclose(outFile);
        printf("Now Closing Output File: %s\n", filename);
    }
    else
    {
        printf("Couldn't open: %s\n", filename);
        return 0;
    }
    return 1;
}

int loadIR(char* filename)
{
    FILE* inFile = fopen(filename, "rb");
    int subChunk2SizeIR;

    if (inFile != NULL)
    {
        printf("Now Reading IR file:  %s\n", filename);

      	//Seeks and reads
        int IRsubChunk1Size;
        fseek(inFile, 16, SEEK_SET);
        fread(&IRsubChunk1Size, 1, 4, inFile);

        if(IRsubChunk1Size == 18)
        {
            short empty;
            fread(&empty, 1, 2, inFile);
        }


        short bits_Per_SampleIR;
        fseek(inFile, 34, SEEK_SET);
        fread(&bits_Per_SampleIR, 1, 2, inFile);


        if (IRsubChunk1Size == 18)
        {
            fseek(inFile, 42, SEEK_SET);
            fread(&subChunk2SizeIR, 1, 4, inFile);
            fseek(inFile, 46, SEEK_SET);
        }
        else
        {
            fseek(inFile, 40, SEEK_SET);
            fread(&subChunk2SizeIR, 1, 4, inFile);
            fseek(inFile, 44, SEEK_SET);
        }

        int bytes_Per_SampleIR = bits_Per_SampleIR / 8;
        number_SamplesIR = subChunk2SizeIR / bytes_Per_SampleIR;

        dataIR = (short*) malloc(sizeof(short) * number_SamplesIR);
        int i = 0;
        short sample = 0;
        while (fread(&sample, 1, bytes_Per_SampleIR, inFile) == bytes_Per_SampleIR)
        {
            dataIR[i++] = sample;
            sample = 0;
        }
    }
    else
    {
        printf("Couldn't open: %s\n", filename);
        return 0;
    }
    return 1;
}


int main(int argc, char* argv[])
{
    clock_t begin, end;
    double time_spent;
    begin = clock();

    //make sure user input is correct
    if (argc != 4)
    {
        printf("Incorrect number of arguments entered. Must follow 'input file' 'IR file' 'output file'.");
        exit(-1);
    }

    char* inputFile = argv[1];
    if (loadWave(inputFile))
    {
			chunkID[5] = '\0';
			format[5] = '\0';
			subChunk1ID[5] = '\0';
			subChunk2ID[5] = '\0';

			printf("\n -------------------------- HEADER INFO ----------------------- \n");
			printf("chunkID:%s\n", chunkID);
			printf("chunkSize:%d\n", chunkSize);
			printf("format:%s\n", format);
			printf("subChunk1ID:%s\n", subChunk1ID);
			printf("subChunk1Size:%d\n", subChunk1Size);
			printf("audioFormat: %d\n", audio_Format);
			printf("numChannels:%d\n", number_Channels);
			printf("sampleRate:%d\n", sample_Rate);
			printf("byteRate:%d\n", byte_Rate);
			printf("blockAlign:%d\n", block_Align);
			printf("bitsPerSample:%d\n", bits_Per_Sample);
			printf("subChunk2ID:%s\n", subChunk2ID);
			printf("subChunk2Size:%d\n\n", subChunk2Size);
    }

    char* IRfile = argv[2];
    loadIR(IRfile);

    saveWave("out.wav");
    end = clock();
    time_spent = (double)(end - begin);
    printf("Time spent: %f seconds", (end - begin) / (double)CLOCKS_PER_SEC);
    free(data);
}
