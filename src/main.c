#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#define RED RGBA16(255, 0, 0, 0)
#define WHITE RGBA16(255, 255, 255, 0)
#define BLACK RGBA16(0, 0, 0, 0)
#define GREEN RGBA16(0, 255, 0, 0)
#define BLUE RGBA16(0, 0, 255, 0)
#define PURPLE RGBA16(128, 0, 128, 0)
#define ORANGE RGBA16(256, 165, 0, 0)
#define DARK_GREY RGBA16(128, 128, 128, 0)
#define SPEED 1

// Mixer channel allocation
#define CHANNEL_SFX    0
#define CHANNEL_MUSIC  1

typedef struct coord {
    int x;
    int y;
} coord;

typedef struct coordSnake {
    int x;
    int y;
    int direction;
} coordSnake;

void rdpq_draw_one_rectangle(int *x, int *y, int *w, int *h, color_t color){
    //set rdp to primitive mode
    rdpq_set_mode_fill(color);

    //check that rdp is ready for new commands - done automatically now?
    //rdpq_sync(SYNC_PIPE);

    //this function can be used to change the color after being init by set_mode_fill
    //rdpq_set_fill_color(color);

    //draw rectangle
    rdpq_fill_rectangle(*x, *y, *x + *w, *y + *h);
}

void update_snake_positions(struct coord *sLeftBot, struct coord *sLeftTop, struct coord *sRightBot, struct coord *sRightTop,
                      int *snakeHeadx, int *snakeHeady){

    sLeftTop->x = (*snakeHeadx);
    sLeftTop->y = (*snakeHeady);
    sLeftBot->x = (*snakeHeadx);
    sLeftBot->y = (*snakeHeady+5);
    sRightBot->x = (*snakeHeadx+5);
    sRightBot->y = (*snakeHeady+5);
    sRightTop->x = (*snakeHeadx+5);
    sRightTop->y = (*snakeHeady);
}

void test_collisions(struct coord *sLeftBot, struct coord *sLeftTop, struct coord *sRightBot, struct coord *sRightTop,
                     struct coord *fLeftBot, struct coord *fLeftTop, struct coord *fRightBot, struct coord *fRightTop,
                     struct coordSnake *snakeBod, struct coordSnake *snakeHead, int *snakeLength, bool *spawnFood, wav64_t *laser)
{
    bool collision = false;

    //---------------------------------------------------------------------------------------Snake -> Food
    //-------left and right sides--------

    // Test the top left corncer of the first box to the second's left side
    // (Is the corner in or on the second box?)
    if((sLeftTop->x <= fRightTop->x) && (sLeftTop->x >= fRightBot->x) &&
       (sLeftTop->y >= fRightTop->y) && (sLeftTop->y <= fRightBot->y))
    {
        collision = true;
    }
    
    // Test the top right corner of the first box to the second's right side
    // (Is the corner in or on the second box?)
    else if((sRightTop->x <= fLeftTop->x) && (sRightTop->x >= fLeftBot->x) &&
            (sRightTop->y >= fLeftTop->y) && (sRightTop->y <= fLeftBot->y))
    {
        collision = true;
    }

    // Test the bottom left corner of the first box to the second's right side
    // (Is the corner in or on the second box?)
    else if((sLeftBot->x <= fRightTop->x) && (sLeftBot->x >= fRightBot->x) &&
            (sLeftBot->y >= fRightTop->y) && (sLeftBot->y <= fRightBot->y))
    {
        collision = true;
    }

    // Test the bottom right corner of the first box to the second's left side
    // (Is the corner in or on the second box?)
    else if((sRightBot->x <= fLeftTop->x) && (sRightBot->x >= fLeftBot->x) &&
            (sRightBot->y >= fLeftTop->y) && (sRightBot->y <= fLeftBot->y))
    {
        collision = true;
    }

    //-------top and bottom--------

    // Test the top left corncer of the first box to the second's bottom side
    // (Is the corner in or on the second box?)
    else if((sLeftTop->x <= fRightBot->x) && (sLeftTop->x >= fLeftBot->x) &&
            (sLeftTop->y <= fRightBot->y) && (sLeftTop->y >= fRightTop->y))
    {
        collision = true;
    }
    
    // Test the top right corner of the first box to the second's bottom side
    // (Is the corner in or on the second box?)
    else if((sRightTop->x <= fRightBot->x) && (sRightTop->x >= fLeftBot->x) &&
            (sRightTop->y <= fRightBot->y) && (sRightTop->y >= fLeftTop->y))
    {
        collision = true;
    }

    // Test the bottom left corner of the first box to the second's top side
    // (Is the corner in or on the second box?)
    else if((sLeftBot->x <= fRightTop->x) && (sLeftBot->x >= fLeftTop->x) &&
            (sLeftBot->y >= fRightTop->y) && (sLeftBot->y <= fRightBot->y))
    {
        collision = true;
    }

    // Test the bottom right corner of the first box to the second's top side
    // (Is the corner in or on the second box?)
    else if((sRightBot->x <= fRightTop->x) && (sRightBot->x >= fLeftTop->x) &&
            (sRightBot->y >= fRightTop->y) && (sRightBot->y <= fLeftBot->y))
    {
        collision = true;
    }
    
    //---------------------------------------------------------------------------------------Food -> Snake
    //-------left and right sides--------

    // Test the top left corner of the second box to the first box
    // (Is the corner in or on the first box?)
    else if((fLeftTop->x <= sRightTop->x) && (fLeftTop->x >= sRightBot->x) &&
            (fLeftTop->y >= sRightTop->y) && (fLeftTop->y <= sRightBot->y))
    {
        collision = true;
    }
    
    // Text the top right corner of the second box to the first box
    // (Is the corner in or on the second box?)
    else if((fRightTop->x <= sLeftTop->x) && (fRightTop->x >= sLeftBot->x) &&
            (fRightTop->y >= sLeftTop->y) && (fRightTop->y <= sLeftBot->y))
    {
        collision = true;
    }
    
    // Test the bottom left corner of the second box to the first box
    // (Is the corner in or on the second box?)
    else if((fLeftBot->x <= sRightTop->x) && (fLeftBot->x >= sRightBot->x) &&
            (fLeftBot->y >= sRightTop->y) && (fLeftBot->y <= sRightBot->y))
    {
        collision = true;
    }

    // Test the bottom right corner of the second box to the first box
    // (Is the corner in or on the second box?)
    else if((fRightBot->x <= sLeftTop->x) && (fRightBot->x >= sLeftBot->x) &&
            (fRightBot->y >= sLeftTop->y) && (fRightBot->y <= sLeftBot->y))
    {
        collision = true;
    }
    
    //-------top and bottom--------

    // Test the top left corncer of the first box to the second's bottom side
    // (Is the corner in or on the second box?)
    else if((fLeftTop->x <= sRightBot->x) && (fLeftTop->x >= sLeftBot->x) &&
            (fLeftTop->y <= sRightBot->y) && (fLeftTop->y >= sRightTop->y))
    {
        collision = true;
    }
    
    // Test the top right corner of the first box to the second's bottom side
    // (Is the corner in or on the second box?)
    else if((fRightTop->x <= sRightBot->x) && (fRightTop->x >= sLeftBot->x) &&
            (fRightTop->y <= sRightBot->y) && (fRightTop->y >= sLeftTop->y))
    {
        collision = true;
    }

    // Test the bottom left corner of the first box to the second's top side
    // (Is the corner in or on the second box?)
    else if((fLeftBot->x <= sRightTop->x) && (fLeftBot->x >= sLeftTop->x) &&
            (fLeftBot->y >= sRightTop->y) && (fLeftBot->y <= sRightBot->y))
    {
        collision = true;
    }

    // Test the bottom right corner of the first box to the second's top side
    // (Is the corner in or on the second box?)
    else if((fRightBot->x <= sRightTop->x) && (fRightBot->x >= sLeftTop->x) &&
            (fRightBot->y >= sRightTop->y) && (fRightBot->y <= sLeftBot->y))
    {
        collision = true;
    }

    //---------------------------------------------------------------------------------------Snake -> Itself
    for(int i = 30; i < (*snakeLength-5); i++){
        //bot left corner of snake block
        if (snakeHead->x == snakeBod[i+5].x && snakeHead->y == snakeBod[i+5].y){
            snakeHead->direction = -1;
            break;
        }
        else if (*snakeLength > 29){
            //bot right corner of snake block
            if(snakeHead->x == (snakeBod[i+5].x+5) && snakeHead->y == snakeBod[i+5].y){
                snakeHead->direction = -1;
                break;
            }
            //top left corner of snake block
            else if (snakeHead->x == snakeBod[i+5].x && snakeHead->y == (snakeBod[i+5].y-5)){
                snakeHead->direction = -1;
                break;
            }
            //top right corner of snake block
            else if (snakeHead->x == (snakeBod[i+5].x+5) && snakeHead->y == (snakeBod[i+5].y-5)){
                snakeHead->direction = -1;
                break;
            }
        }   
    }

    if(collision){
        wav64_play(laser, CHANNEL_SFX);
        *spawnFood = true;
        *snakeLength += 5;
    }
}

void reset_snake(struct coord *sLeftBot, struct coord *sLeftTop, struct coord *sRightBot, struct coord *sRightTop,
                 int *snakeLength, struct coordSnake *snakeHead){
    snakeHead->x = 160;
    snakeHead->y = 120;
    snakeHead->direction = 1;
    sLeftTop->x = (snakeHead->x);
    sLeftTop->y = (snakeHead->y);
    sLeftBot->x = (snakeHead->x);
    sLeftBot->y = (snakeHead->y+5);
    sRightBot->x = (snakeHead->x+5);
    sRightBot->y = (snakeHead->y+5);
    sRightTop->x = (snakeHead->x+5);
    sRightTop->y = (snakeHead->y);
    *snakeLength = 1;
}

void spawn_food(struct coord *fLeftBot, struct coord *fLeftTop, struct coord *fRightBot, struct coord *fRightTop,
                int *foodX, int *foodY, bool reset_flag){
    *foodX = (rand() % 300) + 5;
    *foodY = (rand() % 220) + 5;     
    if(reset_flag){
        //check so food doesnt spawn at start location
        while((*foodX == 160) && (*foodY == 120)){
            *foodX = (rand() % 300) + 5;
            *foodY = (rand() % 220) + 5;     
        }
    }
    fLeftTop->x = (*foodX);
    fLeftTop->y = (*foodY);
    fLeftBot->x = (*foodX);
    fLeftBot->y = (*foodY+5);
    fRightBot->x = (*foodX+5);
    fRightBot->y = (*foodY+5);
    fRightTop->x = (*foodX+5);
    fRightTop->y = (*foodY);
}

//custom functions for reading sprite. bypasses library functions to do so
int filesize( FILE *pFile )
{
    fseek( pFile, 0, SEEK_END );
    int lSize = ftell( pFile );
    rewind( pFile );

    return lSize;
}

sprite_t *read_sprite( const char * const spritename )
{
    FILE *fp = fopen( spritename, "r" );

    if( fp )
    {
        sprite_t *sp = malloc( filesize( fp ) );
        fread( sp, 1, filesize( fp ), fp );
        fclose( fp );

        return sp;
    }
    else
    {
        return 0;
    }
}

void update_anim_counter(float *animX, int *animY, int *animDirection, int *animHeight) {
    //going left
    if(*animDirection == 0){
        if(*animX > -30.0){
            *animX -= 0.5;
        } else {
            *animDirection = rand() % 2;
            *animHeight = rand() % 2;
            if(*animDirection == 0){
                *animX = 400.0;
            } else {
                *animX = -200.0;
            }
            if(*animHeight == 0){
                *animY = 160;
            } else {
                *animY = 70;
            }
        }
    }
    //going right
    if(*animDirection == 1){
        if(*animX < 340.0){
            *animX += 0.5;
        } else {
            *animDirection = rand() % 2;
            *animHeight = rand() % 2;
            if(*animDirection == 0){
                *animX = 400.0;
            } else {
                *animX = -200.0;
            }
            if(*animHeight == 0){
                *animY = 160;
            } else {
                *animY = 70;
            }
        }
    }
}

static volatile uint32_t animcounter = 0;

void update_counter( int ovfl )
{
    animcounter++;
}

int high_score = 1;

int main(void)
{
    //initialize console
    console_init();

    //enable debug flags
    debug_init_usblog();
    console_set_debug(true);

    //initialize controllers
    controller_init();

    //initialize display
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    //initialize dfs
    dfs_init( DFS_DEFAULT_LOCATION );

    //initialize rdp
    rdpq_init();

    //initialize audio
    audio_init(44100, 4);
    mixer_init(16);

    //game state
    //MAIN MENU = 0, GAME = 1, PAUSE = 2
    int game_state = 0;
    uint32_t cur_bg_color = graphics_convert_color(DARK_GREY);

    //snake head + body values
    //snake body (max size to fit in game area = 320x240 / 25)
    struct coordSnake snake[15360];
    struct coordSnake *snakeHead = &snake[0];
    //init snake array
    for(int i=0; i < 15360; i++){
        snake[i].x = -5;
        snake[i].y = -5;
    }
    snakeHead->x = 160;
    snakeHead->y = 120;
    snakeHead->direction = 1;
    struct coord snakeLeftTop = {.x=snakeHead->x, .y=snakeHead->y};
    struct coord snakeLeftBot = {.x=snakeHead->x, .y=(snakeHead->y+5)};
    struct coord snakeRightTop = {.x=(snakeHead->x+5), .y=snakeHead->y};
    struct coord snakeRightBot = {.x=(snakeHead->x+5), .y=(snakeHead->y+5)};
    int height = 5;
    int width = 5;
    int snakeLength = 1;
    int prevPosX = 0;
    int prevPosY = 0;
    int curPosX = 0;
    int curPosY = 0;
    int prevDirection = 0;
    int curDirection = 0;
    int prePauseDirection = 0;

    //food vars
    timer_init();
    srand(timer_ticks());
    bool spawnFood = false;
    int foodX = 0;
    int foodY = 0;
    struct coord foodLeftTop = {.x=foodX, .y=foodY};
    struct coord foodLeftBot = {.x=foodX, .y=(foodY+5)};
    struct coord foodRightTop = {.x=(foodX+5), .y=foodY};
    struct coord foodRightBot = {.x=(foodX+5), .y=(foodY+5)};
    spawn_food(&foodLeftBot, &foodLeftTop, &foodRightBot, &foodRightTop, &foodX, &foodY, true);

    //ness sprite
    int fp = dfs_open("/earthbound.sprite");
    sprite_t *ness = malloc( dfs_size( fp ) );
    dfs_read( ness, 1, dfs_size( fp ), fp );
    dfs_close( fp );

    int animDirection = rand() % 2;
    int animHeight = rand() % 2;
    float animX = 400.0;
    int animY = 70;
    if(animDirection == 0){
        animX = -200.0;
    } else {
        animX = 400.0;
    }

    // Kick off animation update timer to fire thirty times a second
    new_timer(TIMER_TICKS(1000000 / 30), TF_CONTINUOUS, update_counter);

    //open audio files
    //makefile will convert wav audio files in the asset folder to wav64 into the filesystem folder automatically
    wav64_t theme, laser;
    wav64_open(&theme, "rom:/theme.wav64");
    wav64_set_loop(&theme, true);
    wav64_play(&theme, CHANNEL_MUSIC);
    wav64_open(&laser, "rom:/laser.wav64");

    //GAME LOOP 
    while(true){
        //MAIN MENU
        while(game_state == 0){
            //DRAW CURRENT FRAME
            //get display
            surface_t *disp = display_get();

            //draw background
            graphics_fill_screen(disp, cur_bg_color);

            //draw current frame of ness sprite via software
            update_anim_counter(&animX, &animY, &animDirection, &animHeight);
            //graphics_draw_sprite(disp, 82, 160, ness); //sprite sheet
            //graphics_draw_sprite_trans_stride( disp, 50, 100, ness, ((animcounter / 8) & 0x7) * 2 );
            if(animDirection == 0){
                graphics_draw_sprite_trans_stride( disp, animX, animY, ness, ((animcounter / 5) & 1) ? 4: 5 );
            } else {
                graphics_draw_sprite_trans_stride( disp, animX, animY, ness, ((animcounter / 5) & 1) ? 12: 13 );
            }

            //draw text
            char msg1[100];
            graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
            snprintf(msg1, sizeof(msg1), "%s", "SNAKE 64");
            graphics_draw_text(disp, 120, 10, msg1);

            char msg2[100];
            graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
            snprintf(msg2, sizeof(msg2), "%s", "by Jason C.");
            graphics_draw_text(disp, 110, 30, msg2);

            char msg3[100];
            graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
            snprintf(msg3, sizeof(msg3), "%s", "PRESS START TO PLAY");
            graphics_draw_text(disp, 80, 120, msg3);

            char msg4[100];
            graphics_set_color(graphics_make_color(0xff, 0xa5, 0x00, 0xff), 0);
            snprintf(msg4, sizeof(msg4), "%s", "MUSIC by SHANE PERRY");
            graphics_draw_text(disp, 77, 220, msg4);

            //update display
            display_show(disp);

            //GET CONTROLLER INPUT
            struct controller_data controller_data;

            controller_scan();
            controller_data = get_keys_down();

            //unpause
            if(controller_data.c[0].start){
                game_state = 1;
            }

            // Check whether one audio buffer is ready, otherwise wait for next
            // frame to perform mixing.
            if (audio_can_write()) {    	
                short *buf = audio_write_begin();
                mixer_poll(buf, audio_get_buffer_length());
                audio_write_end();
		    }
        }

        //IN GAME
        while(game_state == 1) {
            //DRAW CURRENT FRAME
            //get display
            surface_t *disp = display_get();

            //attach rdp so we can use rdp functions
            rdpq_attach(disp, NULL);

            //draw background
            graphics_fill_screen(disp, graphics_convert_color(DARK_GREY));

            //draw snake
            for(int i = 0; i < snakeLength; i++){
                rdpq_draw_one_rectangle(&snake[i].x, &snake[i].y, &width, &height, WHITE);
            }

            //draw food
            rdpq_draw_one_rectangle(&foodX, &foodY, &width, &height, RED);

            //detach rdp before updating display
            rdpq_detach();

            //display debug values to screen / death message
            graphics_set_color(graphics_make_color(0xff, 0xff, 0xff, 0xff), 0);
            if(snakeHead->direction < 0){
                if((snakeLength / 5 +1) > high_score){
                    high_score = (snakeLength / 5) + 1;
                }

                char msgScore[100];
                snprintf(msgScore, sizeof(msgScore), "%s %i", "YOUR SCORE:", (snakeLength / 5) + 1);
                graphics_draw_text(disp, 100, 20, msgScore);

                char msgHighScore[100];
                snprintf(msgHighScore, sizeof(msgHighScore), "%s %i", "HIGH SCORE:", high_score);
                graphics_draw_text(disp, 100, 70, msgHighScore);

                char msg[100];
                snprintf(msg, sizeof(msg), "%s", "PRESS START TO TRY AGAIN");
                graphics_draw_text(disp, 60, 160, msg);

                char msg2[100];
                snprintf(msg2, sizeof(msg2), "%s", "PRESS Z TO RETURN TO MAIN MENU");
                graphics_draw_text(disp, 40, 210, msg2);
            }

            //update display
            display_show(disp);

            //GET CONTROLLER INPUT
            //move square based on controller dpad (IJKL) input
            struct controller_data controller_data;

            controller_scan();
            controller_data = get_keys_down();

            if(controller_data.c[0].left && snakeHead->direction != -1){
                prevDirection = snakeHead->direction;
                if(prevDirection != 2){
                    snakeHead->direction = 1;
                }
            }
            if(controller_data.c[0].right && snakeHead->direction != -1){
                prevDirection = snakeHead->direction;
                if(prevDirection != 1){
                    snakeHead->direction = 2;
                }
            }
            if(controller_data.c[0].up && snakeHead->direction != -1){
                prevDirection = snakeHead->direction;
                if(prevDirection != 4){
                    snakeHead->direction = 3;
                }
            }            
            if(controller_data.c[0].down && snakeHead->direction != -1){
                prevDirection = snakeHead->direction;
                if(prevDirection != 3){
                    snakeHead->direction = 4;
                }
            }
            //pause
            if(controller_data.c[0].start && snakeHead->direction != -1){
                prePauseDirection = snakeHead->direction;
                snakeHead->direction = 0;
                game_state = 2;
            }
            //reset
            if(snakeHead->direction == -1){
                if(controller_data.c[0].start){
                    //clear snake array
                    for(int i=0; i < snakeLength; i++){
                        snake[i].x = -5;
                        snake[i].y = -5;
                    }
                    reset_snake(&snakeLeftBot, &snakeLeftTop, &snakeRightBot, &snakeRightTop,
                                &snakeLength, snakeHead);
                    spawn_food(&foodLeftBot, &foodLeftTop, &foodRightBot, &foodRightTop, &foodX, &foodY, true);
                } else if(controller_data.c[0].Z){
                    //clear snake array
                    for(int i=0; i < snakeLength; i++){
                        snake[i].x = -5;
                        snake[i].y = -5;
                    }
                    reset_snake(&snakeLeftBot, &snakeLeftTop, &snakeRightBot, &snakeRightTop,
                                &snakeLength, snakeHead);
                    spawn_food(&foodLeftBot, &foodLeftTop, &foodRightBot, &foodRightTop, &foodX, &foodY, true);

                    //return to main menu
                    game_state = 0;
                }
            }
            
            //set speed into appropriate direction
            if(snakeHead->x < 315 && snakeHead->x > 0 && snakeHead->y < 232 && snakeHead->y > 0){
                if(snakeHead->direction == 1){
                    snakeHead->x -= SPEED;
                }
                if(snakeHead->direction == 2){
                    snakeHead->x += SPEED;
                }
                if(snakeHead->direction == 3){
                    snakeHead->y -= SPEED;
                }   
                if(snakeHead->direction == 4){
                    snakeHead->y += SPEED;
                }
                if(snakeHead->direction == 0){
                    continue;
                }
            } else {
                //death by border
                snakeHead->direction = -1;
            }

            //GAME LOGIC
            if(snakeHead->direction > -1){
                //update snake head position
                update_snake_positions(&snakeLeftBot, &snakeLeftTop, &snakeRightBot, &snakeRightTop,
                                    &snakeHead->x, &snakeHead->y);
                
                //collision check
                test_collisions(&snakeLeftBot, &snakeLeftTop, &snakeRightBot, &snakeRightTop, 
                                &foodLeftBot, &foodLeftTop, &foodRightBot, &foodRightTop,
                                snake, snakeHead, &snakeLength, &spawnFood, &laser);

                //update snake body positions
                prevPosX = snake[0].x;
                prevPosY = snake[0].y;
                prevDirection = snake[0].direction;
                for(int j = 1; j < snakeLength; j++){
                    curPosX = snake[j].x;
                    curPosY = snake[j].y;
                    curDirection = snake[j].direction;
                    
                    snake[j].x = prevPosX;
                    snake[j].y = prevPosY;
                    snake[j].direction = prevDirection;

                    prevPosX = curPosX;
                    prevPosY = curPosY;
                    prevDirection = curDirection;
                } 

                //spawn new food if eaten
                if(spawnFood){
                    spawn_food(&foodLeftBot, &foodLeftTop, &foodRightBot, &foodRightTop, &foodX, &foodY, false);
                    spawnFood = false;
                }

                wait_ms(1);
            }

            // Check whether one audio buffer is ready, otherwise wait for next
            // frame to perform mixing.
            if (audio_can_write()) {    	
                short *buf = audio_write_begin();
                mixer_poll(buf, audio_get_buffer_length());
                audio_write_end();
            }
        }

        //PAUSE SCREEN
        while(game_state == 2) {
            //DRAW CURRENT FRAME
            //get display
            surface_t *disp = display_get();

            //attach rdp so we can use rdp functions
            rdpq_attach(disp, NULL);

            //draw background
            graphics_fill_screen(disp, graphics_convert_color(DARK_GREY));

            //draw snake
            for(int i = 0; i < snakeLength; i++){
                rdpq_draw_one_rectangle(&snake[i].x, &snake[i].y, &width, &height, WHITE);
            }

            //draw food
            rdpq_draw_one_rectangle(&foodX, &foodY, &width, &height, RED);

            //detach rdp before updating display
            rdpq_detach();

            //display snakeHead.x and snakeHead.y current values to screen / death message
            graphics_set_color(graphics_make_color(0xff, 0xff, 0xff, 0xff), 0);
            char msg2[100];
            snprintf(msg2, sizeof(msg2), "%s", "PAUSE");
            graphics_draw_text(disp, 140, 10, msg2);

            //update display
            display_show(disp);

            //GET CONTROLLER INPUT
            struct controller_data controller_data;

            controller_scan();
            controller_data = get_keys_down();

            //unpause
            if(controller_data.c[0].start && snakeHead->direction != -1){
                snakeHead->direction = prePauseDirection;
                game_state = 1;
            }
        }
    }
}