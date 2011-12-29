#include <windows.h>
#include <WINBASE.H>
#include <winioctl.h>
#include <stdio.h>
#include <time.h>
#include <api/ntddvdeo.h>

int parseTime(char* time)
{
	double _time = atof(time);
	//to milliseconds
	_time = _time * 3600.0 * 1000.0;
	return _time;
}

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		double timeToSet = parseTime(argv[1]);
		double startTime = parseTime(argv[2]);
		double waitInterval = parseTime(argv[3]);

		//take off a half hour for where to start if less than half hour
		if (timeToSet > startTime)
		{
			printf("beginning sleep\n");
			PostMessage( //turn off monitor
				HWND_BROADCAST,
				WM_SYSCOMMAND,
				SC_MONITORPOWER,
				(LPARAM) 2
				);
			Sleep(timeToSet - startTime);
			printf("ending sleep\n");

			printf("starting the madness\n");
			PostMessage( //turn on monitor
				HWND_BROADCAST,
				WM_SYSCOMMAND,
				SC_MONITORPOWER,
				(LPARAM) -1
				);
			double currentTime = timeToSet - startTime;
			//setup for accessing device
			HANDLE h = CreateFile(
				TEXT("\\\\.\\LCD"), 
				GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_EXISTING, 
				0, 
				NULL
			);
			LPVOID outBuff[256];
			DWORD bytesReturned;
			DWORD qResult;
			TCHAR targetPath[MAX_PATH];
			BOOL err = 0;
			err = DeviceIoControl(
				h,
				IOCTL_VIDEO_QUERY_SUPPORTED_BRIGHTNESS,
				NULL,
				0,
				outBuff,
				sizeof(outBuff),
				&bytesReturned,
				NULL
				);

			UCHAR levels[14];

			memcpy(&levels, &outBuff, sizeof(char)*14);

			DISPLAY_BRIGHTNESS d;
			d.ucDisplayPolicy = DISPLAYPOLICY_BOTH;
			if( err == 0 )
			{
				printf("Oh Shit: %d", GetLastError());
				return 0;
			}

			printf("%d bytes returned\n", bytesReturned);

			for(int i = 13; i > -1; i--)
			{
				printf("setting to %d", levels[i]);
				d.ucACBrightness = levels[i];
				d.ucDCBrightness = levels[i];
				DWORD returned;

				err = DeviceIoControl(
					h,
					IOCTL_VIDEO_SET_DISPLAY_BRIGHTNESS,
					&d,
					sizeof(d),
					NULL,
					0,
					&returned,
					NULL
					);

				printf("%d returned during setting\n", returned);
				if (err == 0)
				{
					printf("oh shit: %d\n", GetLastError());
				}
				Sleep(waitInterval);
			}

			system("C:\\Users\\ag\\AppData\\Local\\Google\\Chrome\\Application\\chrome.exe http://www.youtube.com/watch?v=KV_jhiYW1Ho");
			/*
			while(currentTime < timeToSet)
			{
				currentTime += waitInterval;
				printf("increased\n");
				//increase brightness
				Sleep(waitInterval);
			}
			*/
		}
	}


	return 0;
}