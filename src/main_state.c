#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>

#include <game_constants.h>

static rafgl_raster_t image;
static rafgl_raster_t house;
static rafgl_raster_t upscaled_image;
static rafgl_raster_t raster, raster2;
static rafgl_raster_t checker;

static rafgl_texture_t texture;

static rafgl_spritesheet_t hero;
static rafgl_spritesheet_t hero1;

float time = 0;

#define NUMBER_OF_TILES 27

rafgl_raster_t tiles[NUMBER_OF_TILES];
rafgl_raster_t tilestile1_flag[NUMBER_OF_TILES];

#define WORLD_SIZE 60
int tile_world[WORLD_SIZE][WORLD_SIZE];

#define TILE_SIZE 64

static char save_file[256];
int save_file_no = 0;

int camx = 600, camy = 600;

int selected_x, selected_y;


static int health = 200;

int pressed;
float location = 0;
float selector = 0;



int animation_running = 0;
int animation_frame = 0;
int direction = 0;

int hero_pos_x = RASTER_WIDTH / 2;
int hero_pos_y = RASTER_HEIGHT / 2;

int hero_speed = 300;

int hover_frames = 0;

int flagNegative = 0;

static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;

int original = 0;

typedef struct particle_s
{
    float x, y, dx, dy;
    float life;


} particle_s;

#define MAX_PARTICLES 1000

particle_s particles[MAX_PARTICLES];


typedef struct _ray_t
{
    int x, y, z;
} ray_t;


#define RAY_MAX 220
#define RAY_Z_MAX 2000
ray_t rays[RAY_MAX];


void init_rays(void)
{
    int i;
    for(i = 0; i < RAY_MAX; i++)
    {

        rays[i].z = randf() * RAY_Z_MAX;
        rays[i].x = randf() * RAY_Z_MAX - RAY_Z_MAX / 2;
        rays[i].y = randf() * RAY_Z_MAX - RAY_Z_MAX / 2;
    }
}

void update_rays(int speed)
{
    int i;
    for(i = 0; i < RAY_MAX; i++)
    {
        rays[i].z -= speed/5;
        if(rays[i].z <= 1)
        {
            rays[i].z += RAY_Z_MAX;
            rays[i].x = randf() * RAY_Z_MAX - RAY_Z_MAX / 2;
            rays[i].y = randf() * RAY_Z_MAX - RAY_Z_MAX / 2;
        }
    }
}


void render_rays(rafgl_raster_t *raster, int speed)
{
    int i, brightness;
    float sx0, sy0, sx1, sy1;

    for(i = 0; i < RAY_MAX; i++)
    {
        sx0 = raster_width / 2-10 + rays[i].x * (0.1 * raster_width / rays[i].z);
        sy0 = raster_height / 2 + rays[i].y * (0.1 * raster_height / rays[i].z);

        sx1 = raster_width / 2 + rays[i].x * (0.9 * raster_width / (rays[i].z - speed));
        sy1 = raster_height / 2 + rays[i].y * (0.5 * raster_height / (rays[i].z - speed));

        brightness = 225 - ((float)rays[i].z / RAY_Z_MAX) * 1.0f;

        rafgl_raster_draw_line(raster, sx0, sy0, sx1, sy1, rafgl_RGB(brightness + 30, brightness, 0));

    }

}

void draw_particles(rafgl_raster_t *raster)
{
    int i;

    particle_s s;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        s = particles[i];
        if(s.life <= 0) continue;
        rafgl_raster_draw_line(raster, s.x - s.dx, s.y - s.dy, s.x, s.y, rafgl_RGB(255, 255, 255));
    }
}

void tile1_particles_flag()
{

    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(particles[i].life <= 0) continue;

        particles[i].life-0.001;

        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;
        particles[i].dx *= 0.995f;
        particles[i].dy *= 0.995f;
        particles[i].dy += 0.05;

    }
}

void init_tilemap(void)
{
    int x, y;

    for(y = 0; y < WORLD_SIZE; y++)
    {
        for(x = 0; x < WORLD_SIZE; x++)
        {
            if(randf() > 0.8f)
            {
                tile_world[y][x] = 3 + rand() % 3;
            }
            else
            {
                tile_world[y][x] = rand() % 3;
            }

            // more
            if(y==6 || y==7 || y == 8 || y == 9 || y == 10)
                tile_world[y][x] = 17;

            if(x==58 && y==11)
                tile_world[y][x] = 24;

            if(x==5 && y==5)
                tile_world[y][x] = 18;

            if(x==7 && y==5)
                tile_world[y][x] = 19;

            //ajkuke
            if(x==4 && y==6 || x==20 && y==6 || x==30 && y==6 || x==50 && y==6 ||
               x==1 && y==7 || x==26 && y==7 || x==40 && y==7 || x==56 && y==7 ||
               x==10 && y==8 || x==15 && y==8 || x==23 && y==8|| x==58 && y==8 ||
               x==13 && y==9 || x==29 && y==9 || x==34 && y==9 || x==46 && y==9 ||
               x==5 && y==10 || x==12 && y==10 || x==19 && y==10 || x==27 && y==10 || x==56 && y==10)

                tile_world[y][x] = 23;

            if(x==9 && y==10 || x==22 && y==10 || x==54 && y==10 || x==41 && y==10)
                tile_world[y][x] = 25;

            if(x==59 && y==6 || x==59 && y==7 || x==59 && y==8|| x==59 && y==9 || x==59 && y==10)
                health = 0;

            if(x==0 && y==0)
                tile_world[y][x] = 21;

            if(x==13 && y==2)
                tile_world[y][x] = 22;

           //  if(x==4 && y==3)
          //      tile_world[y][x] = 26;


        }
    }
}

//renderovanje obicne mape
void render_tilemap(rafgl_raster_t *raster)
{

    int x, y;
    int x0 = camx / TILE_SIZE;
    int x1 = x0 + (raster_width / TILE_SIZE) + 1;
    int y0 = camy / TILE_SIZE;
    int y1 = y0 + (raster_height / TILE_SIZE) + 2;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;

    if(x0 >= WORLD_SIZE) x0 = WORLD_SIZE - 1;
    if(y0 >= WORLD_SIZE) y0 = WORLD_SIZE - 1;
    if(x1 >= WORLD_SIZE) x1 = WORLD_SIZE - 1;
    if(y1 >= WORLD_SIZE) y1 = WORLD_SIZE - 1;

    rafgl_raster_t *draw_tile;

     for(y = y0; y <= y1; y++)
    {
        for(x = x0; x <= x1; x++)
        {

              if(tile_world[y][x]==21 || tile_world[y][x]==19 || tile_world[y][x]==26){

                    draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);
                    int i, j, xx;

                    rafgl_pixel_rgb_t sampled, resulting, res;

                    for(j = 0; j < 64; j++)
                    {
                        for(i = 0; i < 64; i++)
                        {
                            // rotate

                            sampled = pixel_at_pm(draw_tile, j, (64 - i - 1) );

                            resulting = sampled;

                            pixel_at_pm(draw_tile, i, j) = resulting;

                            }
                        }
                }else draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);

                if(y == 6 || y == 7 || y == 8 || y == 9 || y == 10){

                    //posterize
                    draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);

                    rafgl_pixel_rgb_t sampled, resulting, res;
                    int i, j;

                      for(j = 0; j < 64; j++)
                      {
                          for(i = 0; i < 64; i++)
                          {

                            sampled = pixel_at_pm(draw_tile, i, j);


                            res.r = sampled.r;
                            res.g = sampled.g / 30 * 30;
                            res.b = sampled.b / 50 * 50;

                            pixel_at_pm(draw_tile, i, j) = res;

                        }}

             }else draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);

            rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE - camx, y * TILE_SIZE - camy - draw_tile->height + TILE_SIZE); // za crtanje mape a imace i kvadratic coveculjak
        }
    }
    rafgl_raster_draw_rectangle(raster, selected_x * TILE_SIZE - camx, selected_y * TILE_SIZE - camy, TILE_SIZE, TILE_SIZE, rafgl_RGB(0, 100, 205));

}

//renderovanje mape2
void render_tilemaptile1_flag(rafgl_raster_t *raster, int flag)
{

    int x, y;
    int x0 = camx / TILE_SIZE;
    int x1 = x0 + (raster_width / TILE_SIZE) + 1;
    int y0 = camy / TILE_SIZE;
    int y1 = y0 + (raster_height / TILE_SIZE) + 2;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;

    if(x0 >= WORLD_SIZE) x0 = WORLD_SIZE - 1;
    if(y0 >= WORLD_SIZE) y0 = WORLD_SIZE - 1;
    if(x1 >= WORLD_SIZE) x1 = WORLD_SIZE - 1;
    if(y1 >= WORLD_SIZE) y1 = WORLD_SIZE - 1;

    rafgl_raster_t *draw_tile;

    if(flag == 0){
        for(y = y0; y <= y1; y++)
        {
            for(x = x0; x <= x1; x++)
            {

                draw_tile = tilestile1_flag + (tile_world[y][x] % NUMBER_OF_TILES);

                rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE - camx, y * TILE_SIZE - camy - draw_tile->height + TILE_SIZE);

            }
        }
        rafgl_raster_draw_circle(raster, selected_x * TILE_SIZE - camx +40, selected_y * TILE_SIZE - camy + 25, TILE_SIZE - 15, rafgl_RGB(255, 255, 200));

    }else if(flag==1) {


        for(y = y0; y <= y1; y++)
        {
            for(x = x0; x <= x1; x++)
            {

                if(tile_world[y][x]==18){

                    draw_tile = tilestile1_flag + (tile_world[y][x] % NUMBER_OF_TILES);
                    int i, j, xx;

                    rafgl_pixel_rgb_t sampled, resulting, res;

                    for(j = 26; j < 55; j++)
                    {
                        for(i = 7; i < 40; i++)
                        {
                            // negativ

                            sampled = pixel_at_pm(draw_tile, i,j);


                                resulting.r = 255 - sampled.r;
                                resulting.g = 255 - sampled.g;
                                resulting.b = 255 - sampled.b;


                           pixel_at_pm(draw_tile, i, j) = resulting;

                            }
                        }

                }else draw_tile = tilestile1_flag + (tile_world[y][x] % NUMBER_OF_TILES);


                rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE - camx, y * TILE_SIZE - camy - draw_tile->height + TILE_SIZE);
            }
            }
        }
       rafgl_raster_draw_circle(raster, selected_x * TILE_SIZE - camx +40, selected_y * TILE_SIZE - camy + 25, TILE_SIZE - 15, rafgl_RGB(255, 255, 255));

}
int tile1_flag = 0; //default


void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    width = raster_width;
    height = raster_height;

    rafgl_raster_load_from_image(&image, "res/images/image.png");
    rafgl_raster_load_from_image(&house, "res/tiles/svgset18.png");

    rafgl_raster_init(&upscaled_image, raster_width, raster_height);
    rafgl_raster_bilinear_upsample(&upscaled_image, &image);

    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);

    rafgl_spritesheet_init(&hero, "res/images/character2.png", 10, 4);

    int i;

    char tile_path[256];

    for(i = 0; i < NUMBER_OF_TILES; i++)
    {
        sprintf(tile_path, "res/tiles1/svgset%d.png", i);
        rafgl_raster_load_from_image(&tilestile1_flag[i], tile_path);
    }

    for(i = 0; i < NUMBER_OF_TILES; i++)
    {
        sprintf(tile_path, "res/tiles/svgset%d.png", i);
        rafgl_raster_load_from_image(&tiles[i], tile_path);
    }


    init_tilemap();
    rafgl_texture_init(&texture);

}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
     if(game_data->keys_down[RAFGL_KEY_SPACE])
        original = 1;
   // else original = 0; // space pressed


    camx = hero_pos_x - RASTER_WIDTH/2;
    camy = hero_pos_y - RASTER_HEIGHT/2;


    if(health <= 0 ){
         hero_pos_x = RASTER_WIDTH/8;  // ako je / 2 i isto za height bice u centru
         hero_pos_y = RASTER_HEIGHT/8;
         camx = hero_pos_x - RASTER_WIDTH/2;
         camy = hero_pos_y - RASTER_HEIGHT/2;

         health=103;

    }

    int star_speed = 5 + 10 * selector/7;
    update_rays(star_speed);



    if((selected_y == 6|| selected_y == 7 || selected_y == 8 || selected_y == 9 || selected_y == 10)){

            if(tile1_flag == 1) {

            hero_pos_x = hero_pos_x + 0.2 /(delta_time / 0.5);
            hero_pos_y = hero_pos_y + 3;

            if(game_data->keys_down[RAFGL_KEY_W]) {
                hero_pos_x = hero_pos_x - 0.2 *(delta_time * 0.5);
                hero_pos_y = hero_pos_y - 3;
            }
            }


            hero_pos_x = hero_pos_x + 0.2 /(delta_time / 0.4); // drugi broj utice na ugao/ kosinu pod kojom ide, prvi znak na smer
            hero_pos_y = hero_pos_y + 4 ;

            if(game_data->keys_down[RAFGL_KEY_W]) {
                hero_pos_x = hero_pos_x - 0.2 *(delta_time * 0.4);
                hero_pos_y = hero_pos_y - 4 ;
            }
            //hero_pos_y = hero_pos_y + 4.5;


            if(tile_world[selected_y][selected_x] == 25) {
                health += 2;
                 printf("Health added on key: %d\n", health);
            }


              if(tile_world[selected_y][selected_x]==23){
                health -= 4;

                printf("Health: %d\n", health);

                   if(health == 0 || health == -1 || health == -2 || health == -3 || health == -4 || health == -5)
                    printf("Game over.\nTry again.\n\n-----------\n");
                    }


    }

    animation_running = 1;
    if(game_data->keys_down[RAFGL_KEY_W])
    {
        hero_pos_y = hero_pos_y - hero_speed * delta_time;

        direction = 2;
    }
    else if(game_data->keys_down[RAFGL_KEY_S])
    {
        hero_pos_y = hero_pos_y + hero_speed * delta_time;
        direction = 0;
    }
    else if(game_data->keys_down[RAFGL_KEY_A])
    {
        hero_pos_x = hero_pos_x - hero_speed * delta_time;
        direction = 1;
    }
    else if(game_data->keys_down[RAFGL_KEY_D])
    {
        hero_pos_x = hero_pos_x + hero_speed * delta_time;
        direction = 3;
    }
    else
    {
        animation_running = 0;
    }

    if(animation_running)
    {
        if(hover_frames == 0)
        {
            animation_frame = (animation_frame + 1) % 10;
            hover_frames = 5;
        }
        else
        {
            hover_frames--;
        }

    }

    selected_x = rafgl_clampi((hero_pos_x + 44)  / (TILE_SIZE), 0, WORLD_SIZE - 1);
    selected_y = rafgl_clampi((hero_pos_y + 44) / (TILE_SIZE), 0, WORLD_SIZE - 1);

    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t sampled, sampled2, resulting, resulting2;


    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;

            sampled = pixel_at_m(upscaled_image, x, y);
            sampled2 = rafgl_point_sample(&image, xn, yn);

            resulting = sampled;
            resulting2 = sampled2;

            resulting.rgba = rafgl_RGB(0, 0, 0);
            resulting = sampled;

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = resulting2;


            if(pressed && rafgl_distance1D(location, y) < 3 && x > raster_width - 15)
            {
                pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
            }

        }
    }




   if(tile_world[selected_y][selected_x] == 22)
     original = 0;

    // na triketri ce se napraviti nova manja i svaki sledeci put coveculjak ce biti vracen unazad na manju
    if(selected_x ==7 && selected_y==5){

        if(tile1_flag == 0) {
            tile_world[2][4] = 26;
           hero_pos_x -= 200;
            hero_pos_y -= 190; // 64 * 3 = 192
           render_tilemap(&raster);
   }
   }




   if(selected_x ==0 && selected_y==0){
        if(tile1_flag == 0) {
          render_rays(&raster, star_speed);

        }else   tile_world[2][4] = 26;

                tile_world[4][1] = 20;  // y, x
                tile_world[5][29] = 20;
                tile_world[0][5] = 20;
                tile_world[12][4] = 20;
                tile_world[1][17] = 20;
                tile_world[4][9] = 20;

   }


   // pokriva krajnje tacke i blago
    if(selected_x == 58 && selected_y == 11 ||
       selected_x == 59 && selected_y == 6 || selected_x == 59 && selected_y == 7 ||
       selected_x == 59 && selected_y == 8 || selected_x == 59 && selected_y == 9 || selected_x == 59 && selected_y == 10)
    {
        health = 0;
        printf("You won!\n\n");
    }



     // na cvet i enter usporava, obican tajl
    if(tile_world[selected_y][selected_x] == 20 && game_data->keys_pressed[RAFGL_KEY_ENTER]){

        hero_speed-=17;  // ako se oduzme npe 60 ide unazad
        printf("Speed down = %d \n", hero_speed);
   }

    // na mini rotaciju i enter ubrzava
    if(tile_world[selected_y][selected_x] == 26 && game_data->keys_pressed[RAFGL_KEY_ENTER]){

        hero_speed+=60;
        printf("Speed up = %d \n", hero_speed);

   }



    if(tile_world[selected_y][selected_x]==19 && game_data->keys_down[RAFGL_KEY_N])
        flagNegative=1;
    else
        flagNegative=0;


    if(game_data->keys_pressed[RAFGL_KEY_INSERT]){
            if(tile1_flag==0)
                    tile1_flag=1;
            else if(tile1_flag==1) {
                tile1_flag=0;
                rafgl_spritesheet_init(&hero, "res/images/character2.png", 10, 4); // svaki put kada se promeni mapa da ostane i karakter koji je za tu mapu
        }
    }

    if(tile1_flag==0){
        render_tilemap(&raster);

    }else if(tile1_flag==1){


        float angle, speed;

        int i=0, radius = 10, gen =1.1; // ako je gen manje, duze traje sneg
        for(i = 0; (i < MAX_PARTICLES) && gen ; i++)
        {
            if(particles[i].life <= 0)
            {
                particles[i].life = 100 * randf() + 100;
                particles[i].x = hero_pos_x;
                particles[i].y = hero_pos_y - 600;

                angle = randf()  *  3.0f;
                speed = (0.2f + 0.7 * randf())*radius;
                particles[i].dx = cosf(angle) * speed;
                particles[i].dy = sinf(angle) * speed;
                gen--;

            }
        }

        render_tilemaptile1_flag(&raster,flagNegative);

        tile1_particles_flag();
        rafgl_spritesheet_init(&hero, "res/images/character1.png", 10, 4);
        draw_particles(&raster);


    }

    //render karaktera
    rafgl_raster_draw_spritesheet(&raster, &hero, animation_frame, direction, hero_pos_x - camx, hero_pos_y - camy);


    /* shift + s snima raster */
    if(game_data->keys_pressed[RAFGL_KEY_S] && game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {
        sprintf(save_file, "save%d.png", save_file_no++);
        rafgl_raster_save_to_png(&raster, save_file);
    }



    /* update-uj teksturu*/
    if(!game_data->keys_pressed[RAFGL_KEY_SPACE])
        rafgl_texture_load_from_raster(&texture, &raster);
   /* else {
     // rafgl_texture_load_from_raster(&texture, &river);
    **/

}

void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_load_from_raster(&texture, original ? &raster : &image);
    rafgl_texture_show(&texture, 0);

}

void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);
}
