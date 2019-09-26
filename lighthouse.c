#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>

#include <math.h>
#include "scum_radio_bsp.h"
#include "lighthouse.h"

//functions//

pulse_type_t classify_pulse(unsigned int timestamp_rise, unsigned int timestamp_fall){
  pulse_type_t pulse_type;
	unsigned int pulse_width;
	
	pulse_width = timestamp_fall - timestamp_rise;
	pulse_type = INVALID;

	// Identify what kind of pulse this was

	if(pulse_width < 300 && pulse_width > 50) pulse_type = LASER; // Laser sweep (THIS NEEDS TUNING)
	if(pulse_width < 665 && pulse_width > 585) pulse_type = AZ; // Azimuth sync, data=0, skip = 0
	if(pulse_width >= 689 && pulse_width < 769) pulse_type = EL; // Elevation sync, data=0, skip = 0
	if(pulse_width >= 793 && pulse_width < 873) pulse_type = AZ; // Azimuth sync, data=1, skip = 0
	if(pulse_width >= 898 && pulse_width < 978) pulse_type = EL; // Elevation sync, data=1, skip = 0
	if(pulse_width >= 978 && pulse_width < 1080) pulse_type = AZ_SKIP; //Azimuth sync, data=0, skip = 1
	if(pulse_width >= 1080 && pulse_width < 1200) pulse_type = EL_SKIP; //elevation sync, data=0, skip = 1
	if(pulse_width >= 1200 && pulse_width < 1300) pulse_type = AZ_SKIP; //Azimuth sync, data=1, skip = 1
	if(pulse_width >= 1300 && pulse_width < 1400) pulse_type = EL_SKIP; //Elevation sync, data=1, skip = 1

	

	return pulse_type;
}

//keeps track of the current state and will print out pulse train information when it's done.
void update_state(pulse_type_t pulse_type, unsigned int timestamp_rise){
	
	static unsigned int azimuth_unknown_sync; 
	static unsigned int azimuth_a_sync;
	static unsigned int azimuth_b_sync;

	static unsigned int azimuth_a_laser;
	static unsigned int azimuth_b_laser;

	static unsigned int elevation_a_sync;
	static unsigned int elevation_b_sync;

	static unsigned int elevation_a_laser;
	static unsigned int elevation_b_laser;	

	static int state = 0;
	
	int nextstate;
	
	if(pulse_type == INVALID){
		return;
	}
	// FSM which searches for the four pulse sequence
			// An output will only be printed if four pulses are found and the sync pulse widths
			// are within the bounds listed above.
	switch(state)
	{
		// Search for an azimuth sync pulse, we don't know if it's A or B yet
		case 0: {
			if(pulse_type == AZ){
				azimuth_unknown_sync = timestamp_rise;
				
				nextstate = 1;
				//printf("state transition: %d to %d\n",state,nextstate);
			}
			else
				nextstate = 0;
			
			break;
		}
		
		// Waiting for another consecutive azimuth sync from B, this should be a skip sync pulse 
		case 1: {
			if(pulse_type == AZ_SKIP) {
				//the last pulse was an azimuth sync from lighthouse A
				azimuth_a_sync = azimuth_unknown_sync;
				
				nextstate = 2;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else
				nextstate = 0;
			//printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			
			break;
		}
		
		// Azimuth laser sweep from lighthouse A
		case 2: {
			if(pulse_type == LASER) {
				azimuth_a_laser = timestamp_rise;
				nextstate = 3;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
				printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}
		
		// Waiting for Elevation A sync with no skip bit 
		case 3:{
			if(pulse_type == EL) {
				elevation_a_sync = timestamp_rise;
				nextstate = 4;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
				printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	

		// Waiting for Elevation B sync with skip 
		case 4:{
			if(pulse_type == EL_SKIP) {
				nextstate = 5;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
				printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	
		// Look for elevation laser pulse
		case 5:{
			if(pulse_type == LASER) {
				elevation_a_laser = timestamp_rise;
				nextstate = 6;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	
		
		//Lighthouse B pulses now//

		// Now look for azimuth A pulse with skip
		case 6:{
			if(pulse_type == AZ_SKIP) {
				nextstate = 7;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	
	
		// Waiting for Azimuth B sync
		case 7:{
			if(pulse_type == AZ) {
				azimuth_b_sync = timestamp_rise;
				nextstate = 8;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	

		// Waiting for Azimuth laser
		case 8:{
			if(pulse_type == LASER) {
				azimuth_b_laser = timestamp_rise;
				nextstate = 9;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			
			break;
		}	

		// Waiting for Elevation A skip
		case 9:{
			if(pulse_type == EL_SKIP) {
				nextstate = 10;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			}
			break;
		}	

		// Waiting for Elevation B sync
		case 10:{
			if(pulse_type == EL) {
				elevation_b_sync = timestamp_rise;
				nextstate = 11;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	

		// Waiting for Elevation laser
		case 11:{
			if(pulse_type == LASER) {
				elevation_b_laser = timestamp_rise;
				nextstate = 12;
				printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
			printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}	

		// If make it to state 12, then have seen sync-skip-sweep-sync-skip-sweep-skip-sync-sweep-skip-sync-sweep
		// Want to make sure that we then see another azimuth sync pulse before printing
		// This should ensure we only see the correct elevation sweep pulse and eliminate glitch printouts
		case 12:{
			
			// Found another azimuth A sync pulse
			if(pulse_type == AZ) {
					
				// Have found all four valid pulses; output data over UART					
				printf("a-%X-%X\n", azimuth_a_laser, azimuth_b_laser);  
				printf("e-%X-%X\n", elevation_a_laser,elevation_b_laser);
				
				// Reset variables

				// Proceed to looking for azimuth sweep pulse
				azimuth_a_sync = timestamp_rise;
				nextstate = 1;
			}
			
			// Found an invalid pulse, start over
			else{
				nextstate = 0;
			}
			break;
		}
	}
	
	state = nextstate;

}

//used for procesing azimuth subsection of pulse train
void azimuth_state(pulse_type_t pulse_type, unsigned int timestamp_rise){
	
	static unsigned int curr_lighthouse;
	
	static unsigned int azimuth_a_sync;
	static unsigned int azimuth_b_sync;

	static unsigned int azimuth_a_laser;
	static unsigned int azimuth_b_laser;
	
	static unsigned int state = 0;
	
}

//keeps track of the current state and will print out pulse train information when it's done.
void update_state_short(pulse_type_t pulse_type, unsigned int timestamp_rise){
	
	static unsigned int azimuth_unknown_sync; 
	static unsigned int azimuth_a_sync;
	static unsigned int azimuth_b_sync;

	static unsigned int azimuth_a_laser;
	static unsigned int azimuth_b_laser;

	static unsigned int elevation_a_sync;
	static unsigned int elevation_b_sync;

	static unsigned int elevation_a_laser;
	static unsigned int elevation_b_laser;	

	static int state = 0;
	
	int nextstate;
	
	if(pulse_type == INVALID){
		return;
	}
	// FSM which searches for the four pulse sequence
			// An output will only be printed if four pulses are found and the sync pulse widths
			// are within the bounds listed above.
	switch(state)
	{
		// Search for an azimuth A sync pulse, we don't know if it's A or B yet
		case 0: {
			if(pulse_type == AZ || pulse_type == AZ_SKIP){
				if(pulse_type == AZ){
					azimuth_unknown_sync = timestamp_rise;
					nextstate = 1;
					//printf("state transition: %d to %d\n",state,nextstate);
				}
				else{
					//go to azimuth b state
					nextstate = 2;
					//printf("state transition: %d to %d\n",state,nextstate);
				}
			}
			else
				nextstate = 0;
			
			break;
		}
		
		// Waiting for another consecutive azimuth skip sync from B, this should be a skip sync pulse 
		case 1: {
			if(pulse_type == AZ_SKIP) {
				
				//lighthouse A sweep pulse
				azimuth_a_sync = timestamp_rise;
				
				nextstate = 3;
				//printf("state transition: %d to %d\n",state,nextstate);
			}
			else
				nextstate = 0;
			//printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			
			break;
		}
		
		// Azimuth B sync state 
		case 2: {
			if(pulse_type == AZ) {
				//the last pulse was an azimuth sync from lighthouse B
				azimuth_b_sync = azimuth_unknown_sync;
				//go to azimuth b laser detect
				nextstate = 4;
				//printf("state transition: %d to %d\n",state,nextstate);
			}
			else{
				nextstate = 0;
				//printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}
	
		// Azimuth A laser sweep
		case 3: {
			if(pulse_type == LASER) {
				//lighthouse a laser
				azimuth_a_laser = timestamp_rise;
				//go to azimuth b laser detect
				nextstate = 0;
				printf("Successful azimuth A: %d, %d\n",azimuth_a_sync,azimuth_a_laser);
			}
			else{
				nextstate = 0;
				printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}
		
		// Azimuth B laser sweep
		case 4: {
			if(pulse_type == LASER) {
				//lighthouse b laser
				azimuth_b_laser = timestamp_rise;
				//go to azimuth b laser detect
				nextstate = 0;
				printf("Successful azimuth A=B: %d, %d\n",azimuth_b_sync,azimuth_b_laser);
			}
			else{
				nextstate = 0;
				printf("state fail. State %d, Pulse Type: %d \n",state,pulse_type);
			}
			break;
		}		

		// If make it to state 12, then have seen sync-skip-sweep-sync-skip-sweep-skip-sync-sweep-skip-sync-sweep
		// Want to make sure that we then see another azimuth sync pulse before printing
		// This should ensure we only see the correct elevation sweep pulse and eliminate glitch printouts
		case 10:{
			
			// Found another azimuth A sync pulse
			if(pulse_type == AZ || pulse_type) {
					
				// Have found all four valid pulses; output data over UART					
				printf("a-%X-%X\n", azimuth_a_laser, azimuth_b_laser);  
				printf("e-%X-%X\n", elevation_a_laser,elevation_b_laser);
				
				// Reset variables

				// Proceed to looking for azimuth sweep pulse
				azimuth_a_sync = timestamp_rise;
				nextstate = 1;
			}
			
			// Found an invalid pulse, start over
			else{
				nextstate = 0;
			}
			break;
		}
	}
	
	state = nextstate;

}

unsigned int sync_pulse_width_compensate(unsigned int pulse_width){
	static unsigned int sync_widths[60];
	unsigned int avg; //average sync pulse width in 10 MHz ticks
	int i;
	unsigned int sum = 0;
	
	static unsigned int sync_count = 0;
	
	//if it's a sync pulse, add to average pulse width
	if(pulse_width > 585 && pulse_width < 700){
		sync_widths[sync_count%60] = pulse_width;
		sync_count++;
	}
	
	for(i = 0; i < 60; i++){
		sum+=sync_widths[i];
	}
	
	avg = sum/60;
	
	printf("avg: %d\n",avg);
	return 	avg;
}