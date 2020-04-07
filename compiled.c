#ifndef DECLRATION_H
#define DECLRATION_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define PINK  0xF81F
#define BLUE  0x001F
#define RED   0xF800

void load_screen();
void clear_screen(bool clear_text_box);
void plot_pixel(int x, int y, short int line_color);
void draw_cursor(int x_cursor, int y_cursor, int colour,int cursor_size, bool left_clicked);
void wait_for_timer_interrupt();
void wait_for_vsync();
void counting_down();
void draw_block(int x_start, int y_start, int colour, int size);
void draw_colour_choice_and_brush_size();

volatile int pixel_buffer_start; // global variable
volatile int* PRIVATE_TIMER_PTR = (int*)0xFFFEC600; //set up a pointer to A9 private timer
volatile int* pixel_ctrl_ptr;
bool left_clicked;

//PS/2 mouse send movement/button information to the host using the 3-byte movementpacket
//This struct is used to store the key information we need from the 3-byte movement packet
typedef struct  {
    int dx;
    int dy;
    int left_pressed_bit;
}mouse_movement;

mouse_movement get_mouse_movement();



#endif



int main(void){
  


  /*************************Initializing front/back pixel buffer ****************************/

  pixel_ctrl_ptr = (int *)0xFF203020; //pointing at the front buffer
  *(pixel_ctrl_ptr + 1) = 0xC8000000; //first store the address in the back buffer

  wait_for_vsync();
  //Initialize the base address to the first pixel in the front buffer(On-chip)
  pixel_buffer_start = *pixel_ctrl_ptr;
  
  //Initializing 320x240 pixels in the front buffer
  load_screen(); // pixel_buffer_start points to the pixel buffer

  
  *(pixel_ctrl_ptr + 1) = 0xC0000000;//set back pixel buffer to start of SDRAM memory
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

  //Initializing 320x240 pixels in the back buffer(SDRAM)
  load_screen();

  /*************************Initializing front/back pixel buffer End****************************/


  //Global signal indicating if the left button on the mouse is pressed
  int cursor_colour = WHITE; //initialize to white brush
  int cursor_size = 4;

  //An array that hold cursor's x/y location in the VGA display
  int cursor_location[2] = {150, 150};
  int prev_buffer_cursor_location[2] = {150, 150};

    
  while(true){
    //@ Need a array to track the previous location(x,y) of the cursor and erase by drawing black on previous
    // clear_screen(1); 
    // Erase by drawing black on the last last previous location () 
    draw_cursor(prev_buffer_cursor_location[0], prev_buffer_cursor_location[1], BLACK, cursor_size, left_clicked);

    mouse_movement movement = get_mouse_movement();
    bool left_clicked = movement.left_pressed_bit;
    
    // Update cursor_location
    cursor_location[0] = cursor_location[0] + movement.dx;
    cursor_location[1] = cursor_location[1] + movement.dy;
    

    //Update cursor size and colour based on cursor location and its clicking
    if(cursor_location[0] >= 20 && cursor_location[0] <= 28 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){ //if clicked on the pink
      cursor_colour = PINK;
    }else if(cursor_location[0] >= 36 && cursor_location[0] <= 44 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){
      cursor_colour = BLUE;
    }else if(cursor_location[0] >= 52 && cursor_location[0] <= 60 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){
      cursor_colour = WHITE;
    }else if(cursor_location[0] >= 20 && cursor_location[0] <= 24 && cursor_location[1] >= 36 && cursor_location[1] <= 40 && left_clicked){
      cursor_size = 4;
    }else if(cursor_location[0] >= 32 && cursor_location[0] <= 40 && cursor_location[1] >= 36 && cursor_location[1] <= 44 && left_clicked){
      cursor_size = 8;
    }else if(cursor_location[0] >= 48 && cursor_location[0] <= 60 && cursor_location[1] >= 36 && cursor_location[1] <= 48 && left_clicked){
      cursor_size = 12;
    }

    counting_down();
    draw_cursor(cursor_location[0], cursor_location[1], cursor_colour, cursor_size, left_clicked);
  
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
    
  }
}



void draw_cursor(int x_cursor, int y_cursor, int colour, int size, bool left_clicked){

      draw_block(x_cursor, y_cursor, colour, size);
      wait_for_vsync(); // swap front and back buffers on VGA vertical sync
      pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

      if(!left_clicked){
        counting_down();
        draw_block(x_cursor, y_cursor, BLUE, size);
        clear_screen(1);
      }
}







void plot_pixel(int x, int y, short int line_color){   
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}



void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int *) 0xFF203020;  //pixel controller of front buffer address
    register int status; //the status register
    
    *pixel_ctrl_ptr = 1;    //start the synchronization procress by writing 
            // the value 1 into the front Buffer register in the pixel buffer controller
    
    status = *(pixel_ctrl_ptr + 3); // Wait for the S bit to become 0, that means VGA sweep is done
    //3*(4 bits) = 12 bits = 0xFF20302C

    //Polling to get status bit
    while ((status & 0x01) != 0) { //wait until bit S of the Status register becomes equal to 0
        status = *(pixel_ctrl_ptr + 3);
    }
}


void draw_block(int x_start, int y_start, int colour, int size){
    for(int x = x_start; x < x_start + size; x++){
        for(int y = y_start; y < y_start + size; y++){
            plot_pixel(x, y, colour);//show cursor
        }
      }
}

void load_screen() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }

    draw_colour_choice_and_brush_size();
    draw_line(50, 100, 50, 200, WHITE);  // vertical line from (100, 100) to (100, 200)
    draw_line(50, 100, 250, 100, WHITE); // hoizontal line from (100, 100) to (200, 100)
    draw_line(250, 100, 250, 200, WHITE); // vertical line from (200, 100) to (200, 200)
    draw_line(50, 200, 250, 200, WHITE); // horizontal line from (200, 100) to (200, 200)
}


void clear_screen(bool clear_text_box) {
  if(!clear_text_box){ //clear the whole screen
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }
  }else{
    for (int x = 51; x < 250; x++) { 
        for (int y = 101; y < 200; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }
  }
}



void counting_down(){
    int status;
    *PRIVATE_TIMER_PTR = 100000000;//load the 200MHz timer to have 0.5s interval
    *(PRIVATE_TIMER_PTR + 2) = 0b011; //Enable is set to 1, load to 1 at the spot of (timer + 8 bits)
    
    while( *(PRIVATE_TIMER_PTR + 3) == 0){ //polling......
        status = *(PRIVATE_TIMER_PTR + 3); //doing nothing
    } // signal registered again
    
    *(PRIVATE_TIMER_PTR + 3) = 0; //reset interrupt signal
}

  
void draw_colour_choice_and_brush_size(){ 
    for(int x = 20; x < 28; x++){ //pink brush
      for(int y = 20; y < 28; y++){
          plot_pixel(x, y, PINK);
      }
    }
  
  for(int x = 36; x < 44; x++){ //blue brush
    for(int y = 20; y < 28; y++){
        plot_pixel(x, y, BLUE);
    }
  }
  
  for(int x = 52; x < 60; x++){ //red brush
    for(int y = 20; y < 28; y++){
        plot_pixel(x, y, RED);
    }
  }

    for(int x = 68; x < 76; x++){ //red brush
      for(int y = 20; y < 28; y++){
          plot_pixel(x, y, WHITE);
      }
    }
  
   for(int x = 20; x < 24; x++){ //4x4 block
        for(int y = 36; y < 40; y++){
            plot_pixel(x, y, WHITE);
        }
    }
  
  for(int x = 32; x < 40; x++){ //8x8 block
        for(int y = 36; y < 44; y++){
            plot_pixel(x, y, WHITE);
        }
    }
  
  for(int x = 48; x < 60; x++){ //12x12 block
        for(int y = 36; y < 48; y++){
            plot_pixel(x, y, WHITE);
        }
    }
}


void draw_line(int x1, int y1, int x2, int y2, short int line_color) {
    
   bool is_steep = abs(y2 - y1) > abs(x2 - x1);
   if(is_steep) {
    swap(&x1, &y1);
    swap(&x2, &y2);

   }

   if(x1 > x2) {
    swap(&x1, &x2);
    swap(&y1, &y2);

   }

   int deltaX = x2 - x1;
   int deltaY = abs(y2 - y1);
    int error = -(deltaX/2);
    int y = y1;
    int y_step = 0;

    if(y1 <y2)
        y_step = 1;
    else
        y_step = -1;

    for(int x = x1; x < (x2+1) ; x++) {
        if(is_steep) 
            plot_pixel(y, x, line_color);
        else
            plot_pixel(x, y, line_color);

        error = error + deltaY;

        if(error >= 0) {
            y = y +y_step;
            error = error - deltaX;
        }
        
    }

}

void swap(int* x, int* y) {
  int temp = *x;
  *x = *y;
  *y = temp;
}


  
mouse_movement get_mouse_movement() {
	//Declare return struct variable
	mouse_movement movement;

	// Up counter resets upon counts to 3 to retrieve one 3-byte movement packet
	int counter = 1;
	// Unsigned char: 1 byte
	int byte1 = 0;
	int byte2 = 0;
	int byte3 = 0;

	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address

    int PS2_data, RVALID;

    while(1) {
        PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field (15th bit)
		if (RVALID && counter == 1) {
			// &0xFF: Only take the first 8 bits - data
			byte1 = PS2_data & 0xFF; 
		} else if (RVALID && counter == 2) {
			byte2 = PS2_data & 0xFF;
		} else if (RVALID && counter == 3) {
			byte3 = PS2_data & 0xFF;
		}



		if (counter ==3) { // One 3-byte movement packet has been stored in byte1/2/3, returning...
			//Get dx/dy movement
			int x_sign_bit = byte1 & 0x10; //0b00010000
			int y_sign_bit = byte1 & 0x20; //0b00100000
			int left_pressed_bit = byte1 & 0x01;
			
			//Constructing movement struct element
			movement.left_pressed_bit = left_pressed_bit;
			
			if(x_sign_bit == 0) { //positive dx
				movement.dx = byte2;
			} else { //negative dx with sign extension
				int sign_extended_twos =   0xFFFFFF00 + byte2 ; // Equivalent to 0xFFFFFE00 + byte 2
				movement.dx = sign_extended_twos;
			}

			if(y_sign_bit == 0) { //positive dx
				movement.dy = byte3;
			} else { //negative dx
				int sign_extended_twos = 0xFFFFFF00 + byte3;
				movement.dy = sign_extended_twos;
			}


			
			printf("x_movement byte (byte 2) is %i \n", movement.dx);
			printf("y_movement byte (byte 3) is %i \n", movement.dy);
			printf("the left-pressed bit is %i \n", left_pressed_bit);

			return movement;

		} else { // Continue reading unfinished 3-byte packet
			counter ++;
		}

		if ( (byte2 == 0xAA) && (byte3 == 0x00) )
		{
			// mouse inserted; initialize sending of data
			*(PS2_ptr) = 0xF4;
		}
		
		
    }

}