#include <raylib.h>
#include <raymath.h>
#include <stdint.h>

#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#define DARK (Color){34, 32, 52, 255}
#define P1_COLOR (Color){69, 40, 60, 255}
#define SCALE 2
#define LOG_MUTE LOG_WARNING
#define LOG_UNMUTE LOG_TRACE

#define DRAWBOX(e) \
    (Rectangle){(e).hitbox.x, (e).hitbox.y, 16 * SCALE, 16 * SCALE}

#define UCLAMP(x, m) ((x) > (m) ? (m) : (x))
#define WRAP_ASSIGN(fptr, fmin, fmax) *fptr = Wrap(*fptr, fmin, fmax)

typedef enum
{
    EAST,
    NORTH,
    WEST,
    SOUTH,
} Direction;

typedef enum
{
    FRAME0,
    FRAME1,
    FRAME2,
} FrameState;

typedef struct
{
    Texture2D sheet;
    Vector2 offset;
    Vector2 offmax;
    Vector2 ssize;
    float speedfactor;
} Sprite;

typedef struct
{
    Rectangle hitbox;
    Sprite sprite;
    float speed;
} Entity;

static inline uint32_t udiv(uint32_t n, uint32_t d)
{
    return n / d;
}

static inline uint32_t umod(uint32_t n, uint32_t d)
{
    return n % d;
}

static inline Rectangle SpriteBox(Sprite sp)
{
    Rectangle rec = {
        umod(sp.offset.x, sp.offmax.x) * sp.ssize.x,
        umod(sp.offset.y, sp.offmax.y) * sp.ssize.y,
        sp.ssize.x,
        sp.ssize.y
    };

    return rec;
} 

int main(void)
{
    SetTraceLogLevel(LOG_MUTE);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "Raylib Test");
    SetTargetFPS(60);

    Image sp_player_base = LoadImage("sp_player.png");
    

    ImageColorContrast(&sp_player_base, 50);
    
    Entity e2 = {
        .hitbox = {
            .x = 300,
            .y = 200,
            .width = 8 * SCALE,
            .height = 16 * SCALE
        },
        .sprite = {
            .sheet = LoadTextureFromImage(sp_player_base),
            .offset = { 1, 3 },
            .offmax = { 3, 4 },
            .ssize = { 16, 16 },
            .speedfactor = 10,
        },
        .speed = 100 * SCALE
    };

    ImageColorReplace(&sp_player_base, BLACK, P1_COLOR);
    
    Entity p1 = {
        .hitbox = {
            .x = 600,
            .y = 400,
            .width = 8 * SCALE,
            .height = 16 * SCALE
        },
        .sprite = {
            .sheet = LoadTextureFromImage(sp_player_base),
            .offset = { 1, 3 },
            .offmax = { 3, 4 },
            .ssize = { 16, 16 },
            .speedfactor = 10,
        },
        .speed = 100 * SCALE
    };

    UnloadImage(sp_player_base);

    SetTraceLogLevel(LOG_UNMUTE);
    SetMousePosition(WIN_WIDTH/2, WIN_HEIGHT/2);

    while (!WindowShouldClose())
    {
        Vector2 vel = {0, 0};
        int idle = 1;

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        {
            vel.y--;
            p1.sprite.offset.y = NORTH;
            idle = 0;
        }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        {
            vel.y++;
            p1.sprite.offset.y = SOUTH;
            idle = 0;
        }
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        {
            vel.x--;
            p1.sprite.offset.y = WEST;
            idle = 0;
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        {
            vel.x++;
            p1.sprite.offset.y = EAST;
            idle = 0;
        }

        if (idle)
        {
            p1.sprite.offset.x = 1;
        }

        vel = Vector2ClampValue(vel, -1, 1);
        vel = Vector2Scale(vel, p1.speed * GetFrameTime());

        p1.hitbox.x += vel.x;
        p1.hitbox.y += vel.y;

        if (vel.x || vel.y)
            p1.sprite.offset.x += p1.speed / p1.sprite.speedfactor
                * GetFrameTime();

        p1.hitbox.x = Clamp(
            p1.hitbox.x,
            -4 * SCALE,
            WIN_WIDTH - (p1.hitbox.width + 4 * SCALE)
        );

        p1.hitbox.y =
            Clamp(p1.hitbox.y, 0, WIN_HEIGHT - p1.hitbox.height);

        BeginDrawing();
            ClearBackground(DARK);

            DrawTexturePro(
                e2.sprite.sheet,
                SpriteBox(e2.sprite),
                DRAWBOX(e2),
                (Vector2){0, 0},
                0,
                WHITE
            );

            DrawTexturePro(
                p1.sprite.sheet,
                SpriteBox(p1.sprite),
                DRAWBOX(p1),
                (Vector2){0, 0},
                0,
                WHITE
            );

            DrawFPS(0, 0);
        EndDrawing();
    }

    SetTraceLogLevel(LOG_MUTE);
    UnloadTexture(p1.sprite.sheet);
    CloseWindow();
    return 0;
}
