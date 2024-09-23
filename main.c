#include <raylib.h>
#include <raymath.h>
#include <stdint.h>

#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#define DARK CLITERAL(Color){34, 32, 52, 255}
#define SCALE 2

#define DRAWBOX(e) \
    (Rectangle){(e).hitbox.x, (e).hitbox.y, 16 * SCALE, 16 * SCALE}
#define SPRITEBOX(e) \
    (Rectangle){((int)((e).animstate / 4)) * 16, (e).facing * 16, 16, 16}

typedef enum
{
    WEST,
    NORTH,
    EAST,
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
    uint8_t facing:2;
    uint8_t state:2;
} FrameVariant;

typedef struct
{
    Texture2D *tex;
    FrameVariant variant;
} SpriteManager;

typedef struct
{
    Rectangle hitbox;
    Direction facing;
    uint8_t animstate;
    uint8_t speed;
} Player;

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "");
    SetTargetFPS(60);

    Image sprite_sheet = LoadImage("sprite.png");
    Texture2D sprite_texture = LoadTextureFromImage(sprite_sheet);
    UnloadImage(sprite_sheet);

    Player player = {
        .hitbox = {
            .x = 600,
            .y = 400,
            .width = 8 * SCALE,
            .height = 16 * SCALE
        },
        .animstate = 0,
        .facing = SOUTH,
        .speed = 200
    };
    
    SetTraceLogLevel(LOG_TRACE);
    while (!WindowShouldClose())
    {
        Vector2 vel = {0, 0};

        if (IsKeyDown(KEY_W))
        {
            vel.y--;
            player.facing = NORTH;
            player.animstate++;
        }
        if (IsKeyDown(KEY_S))
        {
            vel.y++;
            player.facing = SOUTH;
            player.animstate++;
        }
        if (IsKeyDown(KEY_A))
        {
            vel.x--;
            player.facing = EAST;
            player.animstate++;
        }
        if (IsKeyDown(KEY_D))
        {
            vel.x++;
            player.facing = WEST;
            player.animstate++;
        }

        if (player.animstate != 1)
            player.animstate = player.animstate % 8;

        if (
            IsKeyUp(KEY_W) &&
            IsKeyUp(KEY_A) &&
            IsKeyUp(KEY_S) &&
            IsKeyUp(KEY_D)
        ) {
            // player.animstate = 1;
        }

        vel = Vector2ClampValue(vel, -1, 1);
        vel = Vector2Scale(vel, player.speed * GetFrameTime());
        player.hitbox.x += vel.x;
        player.hitbox.y += vel.y;

        player.hitbox.x = Clamp(
            player.hitbox.x,
            -4 * SCALE,
            WIN_WIDTH - (player.hitbox.width + 3 * SCALE)
        );

        player.hitbox.y =
            Clamp(player.hitbox.y, 0, WIN_HEIGHT - player.hitbox.height);

        BeginDrawing();
            ClearBackground(DARK);

            DrawTexturePro(
                sprite_texture,
                SPRITEBOX(player),
                DRAWBOX(player),
                (Vector2){0, 0},
                0,
                WHITE
            );

            DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
