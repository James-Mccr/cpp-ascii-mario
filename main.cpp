#include "lib/console.h"
#include "lib/frame.h"
#include "lib/input.h"
#include "lib/render.h"
#include <cstdlib>
#include <limits>
#include <string>
#include <memory>
#include <vector>

using namespace std;

static constexpr int FRAMES_PER_SECOND {30};

class Grid
{
public:
    Grid(int _width, int _height)
    {
        width = _width;
        height = _height;
        tiles = vector(height, string(width, empty));
    }

    const vector<string>& GetTiles() const { return tiles; }

    void SetTile(int x, int y, char c = empty)
    {
        tiles[y][x] = c;
    }

    bool IsOutOfBounds(int x, int y)
    {
        return x < 0 || x >= width || y < 0 || y >= height;
    }

    bool IsCollision(int x, int y, char c)
    {
        if (IsOutOfBounds(x, y)) return false;
        return tiles[y][x] == c;
    }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    static constexpr char empty = ' ';
    int width{};
    int height{};
    vector<string> tiles;
};

static unique_ptr<Grid> grid;
static UserInput userInput{};

class Zoomba
{
public:
    static constexpr char ascii = 'Z';

    bool IsAlive() const { return alive; }

    void Spawn()
    {
        if (rand() > 1073741823)
        {
            x = -1;
            speed = 1;
        }
        else
        {
            x = grid->GetWidth();
            speed = -1;
        }

        y = grid->GetHeight()-1;
        alive = true;
    }

    void Squash()
    {
        alive = false;
        if (!grid->IsOutOfBounds(x, y))
            grid->SetTile(x, y);
        if (!grid->IsOutOfBounds(x-speed, y))
            grid->SetTile(x-speed, y);
    }

    void Update()
    {
        if (!alive) return;

        if (frames++ < 4) return;        
        frames = 0;

        if (!grid->IsOutOfBounds(x,y))
            grid->SetTile(x, y);
        if (!grid->IsOutOfBounds(x-speed,y))
            grid->SetTile(x-speed, y);

        x += speed;
        if (x == -2 || x == grid->GetWidth()+1)
        {
            alive = false;    
            return;
        }

        grid->SetTile(x, y, ascii);
        if (!grid->IsOutOfBounds(x-speed, y))
            grid->SetTile(x-speed, y, ascii);
    }

private:
    int x{};
    int y{};
    int speed{};
    bool alive{};
    int frames{};
};

class Player
{
public:

    int GetPoints() const { return points; }

    bool Collide()
    {
        if (speed[1] >= 0) return false;
        if (!grid->IsCollision(x, y+1, Zoomba::ascii)) return false;
        bounces++;
        momentum = 0;
        points += bounces;
        speed[1] = INERTIA + (bounces * 5);
        return true;
    }

    void Update()
    {    
        bool grounded = grid->IsOutOfBounds(x, y+1);

        if (grounded)
            bounces = 0;

        if (userInput == UserInput::Left)
            speed[0] = -1;
        else if (userInput == UserInput::Right)
            speed[0] = 1;
        else if (userInput == UserInput::Up && grounded)
            speed[1] = INERTIA;
        else
            speed[0] = 0;

        grid->SetTile(x, y);

        if (!grounded && speed[1] > -30)
            speed[1] -= GRAVITY;
        momentum += speed[1];
        if (momentum > 40 || momentum < -40)
        {
            if (speed[1] > 0)
            {
                if (grid->IsOutOfBounds(x, y-1))
                    hitTheRoof = true;
                else
                    y += -1;
            }
            else if (speed[1] < 0)
            {
                if (grid->IsOutOfBounds(x, y+1))
                    speed[1] = 0;
                else             
                    y += 1;       
            }
                
            momentum = 0;
        }

        if (!grid->IsOutOfBounds(x+speed[0], y))
            x += speed[0];
        
        grid->SetTile(x, y, ascii);
    }

    bool IsVictory() const { return hitTheRoof; }

    static constexpr char ascii = '@';

private:
    int x{15};
    int y{10};
    int momentum{};
    int points{};
    int bounces{};
    bool hitTheRoof{};
    static constexpr int JUMP = -1;
    static constexpr int GRAVITY = 1;
    static constexpr int INERTIA = 20;
    vector<int> speed{0,GRAVITY};
};

class Game
{
public:
    void Update()
    {
        if (player.Collide())
            zoomba.Squash();
        player.Update();

        if (!zoomba.IsAlive())
            zoomba.Spawn();
        else
            zoomba.Update();
    }

    int GetPoints() const { return player.GetPoints(); }
    bool IsVictory() const { return player.IsVictory(); }

private:
    Player player{};
    Zoomba zoomba{};

};

int main()
{
    Console console{};
    Frame frame{30};
    Input input{};
    Render render{console};
    Game game{};
    srand(time(nullptr));

    unique_ptr<Grid> g(new Grid(console.width, console.height));
    grid = std::move(g);

    while(1)
    {
        frame.limit();

        userInput = input.Read();

        if (userInput == UserInput::Quit) return 0;

        game.Update();

        if (game.IsVictory())
        {
            console.moveCursor(grid->GetHeight()/2, grid->GetWidth()/2);
            console.print("You win!");
            console.moveCursor(grid->GetHeight()/2+1, grid->GetWidth()/4);
            console.print("You earned " + std::to_string(game.GetPoints()) + " points!");
            break;
        }

        render.Draw(grid->GetTiles());
    }

    frame = {1};
    frame.limit();
    frame.limit();
    frame.limit();
    frame.limit();
    frame.limit();

    return 0;
}
