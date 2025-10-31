#include "Pathfinding.h"
#include <queue>
#include <algorithm>
#include <limits>


#include <iostream>

struct Node {
    int idx;
    int g;
    int f;
    int parent; // index of parent node
    Node(): idx(-1), g(0), f(0), parent(-1) {}
};

static inline int manhattan(int x1,int y1,int x2,int y2) {
    return abs(x1-x2) + abs(y1-y2);
}

std::vector<std::pair<int,int>> findPathAStar(const std::vector<std::string>& grid,
                                              int startCol, int startRow,
                                              int goalCol, int goalRow)
{
    int rows = (int)grid.size();
    if (rows == 0) return {};
    int cols = (int)grid[0].size();

    auto inBounds = [&](int c,int r){ return c>=0 && c<cols && r>=0 && r<rows; };
    
    // Verifica se uma célula é parede
    auto isWall = [&](int c, int r) -> bool {
        return inBounds(c,r) && grid[r][c] == 'X';
    };
    
    // Verifica se uma célula é caminhável (não é 'X' e está dentro dos limites)
    auto walkable = [&](int c, int r) -> bool {
        if (!inBounds(c,r) || grid[r][c] == 'X') return false;
        return true;
    };

    if (!walkable(startCol,startRow) || !walkable(goalCol,goalRow)) {
        if (!walkable(startCol,startRow)) std::cout << "[A*] start not walkable: ("<<startCol<<","<<startRow<<")\n";
        if (!walkable(goalCol,goalRow)) std::cout << "[A*] goal not walkable: ("<<goalCol<<","<<goalRow<<")\n";
        return {};
    }

    int N = rows * cols;
    std::vector<int> g(N, std::numeric_limits<int>::max());
    std::vector<int> f(N, std::numeric_limits<int>::max());
    std::vector<int> parent(N, -1);
    std::vector<char> closed(N, 0);

    auto idx = [&](int c,int r){ return r*cols + c; };
    int startIdx = idx(startCol,startRow);
    int goalIdx = idx(goalCol,goalRow);

    struct PQItem { int f; int index; };
    struct Cmp { bool operator()(PQItem a, PQItem b) const { return a.f > b.f; } };
    std::priority_queue<PQItem, std::vector<PQItem>, Cmp> open;

    g[startIdx] = 0;
    f[startIdx] = manhattan(startCol,startRow,goalCol,goalRow);
    open.push({f[startIdx], startIdx});

    const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    int found = 0;
    while(!open.empty()) {
        PQItem cur = open.top(); open.pop();
        int u = cur.index;
        if (closed[u]) continue;
        closed[u] = 1;
        if (u == goalIdx) { found = 1; break; }

        int ux = u % cols;
        int uy = u / cols;
        for (int d=0; d<4; ++d) {
            int nx = ux + dirs[d][0];
            int ny = uy + dirs[d][1];
            if (!walkable(nx,ny)) continue;
            int v = idx(nx,ny);
            if (closed[v]) continue;
            int tentative_g = g[u] + 1;
            if (tentative_g < g[v]) {
                g[v] = tentative_g;
                parent[v] = u;
                f[v] = g[v] + manhattan(nx,ny,goalCol,goalRow);
                open.push({f[v], v});
            }
        }
    }

    if (!found) return {};

    // reconstruct path
    std::vector<std::pair<int,int>> path;
    int cur = goalIdx;
    while (cur != -1) {
        int cx = cur % cols;
        int cy = cur / cols;
        path.emplace_back(cx, cy);
        cur = parent[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}
