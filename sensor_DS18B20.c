1.	#include <stdio.h>
2.	#include <stdlib.h>
3.	#include <string.h>
4.	#include <unistd.h>
5.	#include <errno.h>
6.	#include <time.h>
7.	#include <wiringPi.h>
8.	#include <wiringPiI2C.h>
9.	#include <onewire.h>
10.	#include <ds18b20.h>
11.	
12.	#define ONE_WIRE_BUS 4
13.	
14.	int main(void)
15.	{
16.	    int i;
17.	    int err;
18.	    int devCount;
19.	    int temperature;
20.	    double celsius;
21.	
22.	    // Initialize the library
23.	    if (wiringPiSetup() == -1) {
24.	        fprintf(stderr, "wiringPiSetup failed: %s\n", strerror(errno));
25.	        return 1;
26.	    }
27.	
28.	    // Initialize the OneWire library
29.	    if ((err = onewire_init(ONE_WIRE_BUS)) != 0) {
30.	        fprintf(stderr, "onewire_init failed: %s\n", onewire_strerror(err));
31.	        return 1;
32.	    }
33.	
34.	    // Get the number of devices on the OneWire bus
35.	    if ((err = onewire_get_dev_count(&devCount)) != 0) {
36.	        fprintf(stderr, "onewire_get_dev_count failed: %s\n", onewire_strerror(err));
37.	        return 1;
38.	    }
39.	
40.	    // Loop through each device on the OneWire bus
41.	    for (i = 0; i < devCount; i++) {
42.	        // Search for the DS18B20 device
43.	        if (ds18b20_search(i) != 0) {
44.	            fprintf(stderr, "ds18b20_search failed: %s\n", ds18b20_strerror(err));
45.	            return 1;
46.	        }
47.	
48.	        // Read the temperature from the DS18B20 device
49.	        if (ds18b20_read_temp(&temperature) != 0) {
50.	            fprintf(stderr, "ds18b20_read_temp failed: %s\n", ds18b20_strerror(err));
51.	            return 1;
52.	        }
53.	
54.	        // Convert the temperature from integer to double
55.	        celsius = (double)temperature / 1000.0;
56.	
57.	        // Print the temperature in Celsius
58.	        printf("Temperature: %.1f C\n", celsius);
59.	
60.	        // Reset the search state
61.	        ds18b20_reset_search();
62.	    }
63.	
64.	    return 0;
65.	}
