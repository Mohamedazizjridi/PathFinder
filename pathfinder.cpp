#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;
extern int n, m;
extern vector<string> grid;
void read_input_stream(istream &in);
void compute_monster_dist();
set<pair<int,int>> spread_monsters(const vector<string>& grid, const set<pair<int,int>>& monsters);
vector<char> find_safe_path();
pair<int,int> find_start();
extern int dx[];
extern int dy[];
extern vector<vector<int>> monsterDist;
void render_tui_grid(WINDOW* win, const vector<string>& g, pair<int,int> cursor, const set<pair<int,int>>& mons, pair<int,int> player) {
    int rows = g.size();
    int cols = rows?g[0].size():0;
    werase(win);
    box(win, 0, 0);
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
            int wy = 1 + i, wx = 1 + j;
            char ch = g[i][j];
            if (player.first==i && player.second==j) {
                wattron(win, COLOR_PAIR(3)); mvwaddch(win, wy, wx, 'A'); wattroff(win, COLOR_PAIR(3));
            } else if (mons.find({i,j})!=mons.end()) {
                wattron(win, COLOR_PAIR(2)); mvwaddch(win, wy, wx, 'M'); wattroff(win, COLOR_PAIR(2));
            } else if (ch == '#') {
                wattron(win, COLOR_PAIR(1)); mvwaddch(win, wy, wx, '#'); wattroff(win, COLOR_PAIR(1));
            } else {
                mvwaddch(win, wy, wx, ch=='.' ? '.' : ch);
            }
            if (cursor.first==i && cursor.second==j) {
                // highlight cursor
                mvwchgat(win, wy, wx, 1, A_REVERSE, 0, NULL);
            }
        }
    }
    wrefresh(win);
}
void render_tui_play(WINDOW* win, const vector<string>& g, const vector<vector<int>>& monsterDist, int t, const vector<pair<int,int>>& pathCells, pair<int,int> player, bool showPath) {
    int rows = g.size();
    int cols = rows?g[0].size():0;
    werase(win);
    box(win, 0, 0);
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
            int wy = 1 + i, wx = 1 + j;
            char ch = g[i][j];
            if (ch == '#') {
                wattron(win, COLOR_PAIR(1));
                mvwaddch(win, wy, wx, '#');
                wattroff(win, COLOR_PAIR(1));
            } else {
                mvwaddch(win, wy, wx, '.');
            }
        }
    }
    if (showPath) {
        for (size_t idx = 0; idx < pathCells.size(); idx++) {
            int i = pathCells[idx].first;
            int j = pathCells[idx].second;
            int wy = 1 + i, wx = 1 + j;
            
            if (!(player.first == i && player.second == j)) {
                wattron(win, COLOR_PAIR(6));
                mvwaddch(win, wy, wx, '*');
                wattroff(win, COLOR_PAIR(6));
            }
        }
    }
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
            int wy = 1 + i, wx = 1 + j;
            char ch = g[i][j];
            if (ch == 'M') {
                wattron(win, COLOR_PAIR(2) | A_BOLD);
                mvwaddch(win, wy, wx, 'M');
                wattroff(win, COLOR_PAIR(2) | A_BOLD);
            } else if (monsterDist.size() && monsterDist[i][j] <= t) {
                wattron(win, COLOR_PAIR(2));
                mvwaddch(win, wy, wx, '~');
                wattroff(win, COLOR_PAIR(2));
            }
        }
    }
    if (player.first != -1) {
        int wy = 1 + player.first;
        int wx = 1 + player.second;
        wattron(win, COLOR_PAIR(6) | A_BOLD);
        mvwaddch(win, wy, wx, '@');
        wattroff(win, COLOR_PAIR(6) | A_BOLD);
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dy == 0 && dx == 0) continue;
                int ny = wy + dy, nx = wx + dx;
                if (ny > 0 && ny <= rows && nx > 0 && nx <= cols) {
                    chtype existing = mvwinch(win, ny, nx);
                    if ((existing & A_CHARTEXT) == '.') {
                        wattron(win, COLOR_PAIR(6) | A_DIM);
                        mvwaddch(win, ny, nx, '.');
                        wattroff(win, COLOR_PAIR(6) | A_DIM);
                    }
                }
            }
        }
    }
    
    wrefresh(win);
}
void render_tui_play_dynamic(WINDOW* win, const vector<string>& g, const set<pair<int,int>>& monsters, int t, const vector<pair<int,int>>& pathCells, pair<int,int> player, bool showPath) {
    int rows = g.size();
    int cols = rows?g[0].size():0;
    werase(win);
    box(win, 0, 0);
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
            int wy = 1 + i, wx = 1 + j;
            char ch = g[i][j];
            if (ch == '#') {
                wattron(win, COLOR_PAIR(1)); mvwaddch(win, wy, wx, '#'); wattroff(win, COLOR_PAIR(1));
            } else {
                mvwaddch(win, wy, wx, '.');
            }
        }
    }
    if (showPath) {
        for (auto &pc : pathCells) {
            int i = pc.first, j = pc.second;
            if (player.first==i && player.second==j) continue;
            int wy = 1 + i, wx = 1 + j;
            wattron(win, COLOR_PAIR(6)); mvwaddch(win, wy, wx, '*'); wattroff(win, COLOR_PAIR(6));
        }
    }
    for (auto &mp : monsters) {
        int wy = 1 + mp.first, wx = 1 + mp.second;
        wattron(win, COLOR_PAIR(2) | A_BOLD); mvwaddch(win, wy, wx, 'M'); wattroff(win, COLOR_PAIR(2) | A_BOLD);
    }
    if (player.first != -1) {
        int wy = 1 + player.first, wx = 1 + player.second;
        wattron(win, COLOR_PAIR(6) | A_BOLD); mvwaddch(win, wy, wx, '@'); wattroff(win, COLOR_PAIR(6) | A_BOLD);
    }
    wrefresh(win);
}
int tui_game_loop(const string &infile) {
    int rows = 12, cols = 30;
    vector<string> localGrid;
    set<pair<int,int>> monsters;
    pair<int,int> player = {-1,-1};
    if (!infile.empty()) {
        ifstream in(infile);
        if (in) {
            read_input_stream(in);
            localGrid = grid; rows = n; cols = m;
            for (int i=0;i<rows;i++) for (int j=0;j<cols;j++) {
                if (localGrid[i][j]=='M') monsters.insert({i,j});
                if (localGrid[i][j]=='A') player = {i,j};
            }
        }
    }
    if (localGrid.empty()) localGrid.assign(rows, string(cols, '.'));
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    if (has_colors()) { start_color(); use_default_colors(); init_pair(1, COLOR_WHITE, -1); init_pair(2, COLOR_RED, -1); init_pair(3, COLOR_CYAN, -1); init_pair(4, COLOR_YELLOW, -1); init_pair(6, COLOR_GREEN, -1); }
    int win_h = rows + 2, win_w = cols + 2;
    int side_w = 32;
    int scr_h, scr_w; getmaxyx(stdscr, scr_h, scr_w);
    int gy = max(0, (scr_h - win_h)/2), gx = max(0, (scr_w - (win_w+side_w))/2);
    WINDOW* gridWin = newwin(win_h, win_w, gy, gx);
    WINDOW* sideWin = newwin(win_h, side_w, gy, gx + win_w);
    box(sideWin, 0, 0);
    pair<int,int> cursor = {0,0};
    bool running = true;
    int ch;
    while (running) {
        werase(sideWin); box(sideWin, 0, 0);
        mvwprintw(sideWin, 1, 2, "PathFinder++ ");
        mvwprintw(sideWin, 3, 2, "Arrows: move cursor");
        mvwprintw(sideWin, 4, 2, "w: toggle wall");
        mvwprintw(sideWin, 5, 2, "m: toggle monster");
        mvwprintw(sideWin, 6, 2, "a: set player");
        mvwprintw(sideWin, 7, 2, "p: play");
        mvwprintw(sideWin, 8, 2, "s: save  l: load");
        mvwprintw(sideWin, 9, 2, "q: quit");
        mvwprintw(sideWin, 11, 2, "Cursor: %d,%d", cursor.first, cursor.second);
        if (player.first!=-1) mvwprintw(sideWin, 12, 2, "Player: %d,%d", player.first, player.second);
        mvwprintw(sideWin, win_h-2, 2, "Monsters: %zu", monsters.size());
        wrefresh(sideWin);
        render_tui_grid(gridWin, localGrid, cursor, monsters, player);
        ch = wgetch(stdscr);
        if (ch == KEY_UP) { cursor.first = max(0, cursor.first-1); }
        else if (ch == KEY_DOWN) { cursor.first = min(rows-1, cursor.first+1); }
        else if (ch == KEY_LEFT) { cursor.second = max(0, cursor.second-1); }
        else if (ch == KEY_RIGHT) { cursor.second = min(cols-1, cursor.second+1); }
        else if (ch == 'w') { int r=cursor.first, c=cursor.second; localGrid[r][c] = (localGrid[r][c]=='#')?'.':'#'; }
        else if (ch == 'm') { int r=cursor.first, c=cursor.second; if (monsters.find({r,c})!=monsters.end()) monsters.erase({r,c}); else monsters.insert({r,c}); localGrid[r][c]='.'; }
        else if (ch == 'a') { player = cursor; }
        else if (ch == 's') {
            echo(); nocbreak(); mvwprintw(sideWin, win_h-3, 2, "Save file: "); wrefresh(sideWin);
            char fname[256]; mvwgetnstr(sideWin, win_h-3, 14, fname, 255);
            ofstream out(fname);
            if (out) { out << rows << " " << cols << "\n"; for (int i=0;i<rows;i++) out << localGrid[i] << "\n"; mvwprintw(sideWin, win_h-4, 2, "Saved."); }
            else mvwprintw(sideWin, win_h-4, 2, "Failed to save.");
            noecho(); cbreak(); wrefresh(sideWin);
        }
        else if (ch == 'l') {
            echo(); nocbreak(); mvwprintw(sideWin, win_h-3, 2, "Load file: "); wrefresh(sideWin);
            char fname[256]; mvwgetnstr(sideWin, win_h-3, 14, fname, 255);
            ifstream in(fname);
            if (in) {
                read_input_stream(in);
                localGrid = grid; rows = n; cols = m;
                monsters.clear(); player={-1,-1};
                delwin(gridWin); delwin(sideWin);
                win_h = rows + 2; win_w = cols + 2; side_w = 32;
                getmaxyx(stdscr, scr_h, scr_w);
                gy = max(0, (scr_h - win_h)/2); gx = max(0, (scr_w - (win_w+side_w))/2);
                gridWin = newwin(win_h, win_w, gy, gx);
                sideWin = newwin(win_h, side_w, gy, gx + win_w);
                for (int i=0;i<rows;i++) for (int j=0;j<cols;j++) { if (localGrid[i][j]=='M') monsters.insert({i,j}); if (localGrid[i][j]=='A') player={i,j}; }
            } else {
                mvwprintw(sideWin, win_h-4, 2, "Failed to open."); wrefresh(sideWin);
            }
            noecho(); cbreak();
        }
        else if (ch == 'p') {
            n = rows; m = cols; grid = localGrid;
            pair<int,int> startPos = player;
            if (startPos.first == -1) startPos = find_start();
            if (startPos.first != -1) {
                grid[startPos.first][startPos.second] = 'A';
            }
            compute_monster_dist();
            if (startPos.first != -1 && monsterDist[startPos.first][startPos.second] == 0) {
                werase(sideWin); box(sideWin,0,0);
                wattron(sideWin, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(sideWin, 4, 2, "Start position unsafe!");
                mvwprintw(sideWin, 5, 2, "Monster at or adjacent");
                wattroff(sideWin, COLOR_PAIR(2) | A_BOLD);
                mvwprintw(sideWin, 7, 2, "Press any key to continue");
                wrefresh(sideWin);
                nodelay(stdscr, FALSE);
                wgetch(stdscr);
                nodelay(stdscr, TRUE);
                continue;
            }
            auto safePath = find_safe_path();
            vector<pair<int,int>> pathCells;
            if (!safePath.empty()){
                pair<int,int> st = (player.first == -1) ? find_start() : player;
                if (st.first == -1) st = startPos;
                int x = st.first, y = st.second;
                for (char c: safePath) {
                    int dir = 0;
                    if (c=='D') dir = 0;
                    else if (c=='U') dir = 1;
                    else if (c=='R') dir = 2;
                    else if (c=='L') dir = 3;
                    x += dx[dir]; y += dy[dir];
                    pathCells.emplace_back(x,y);
                }
                bool valid = true;
                for (size_t i = 0; i < pathCells.size(); ++i) {
                    int arrive = (int)i + 1;
                    int px = pathCells[i].first, py = pathCells[i].second;
                    if (px < 0 || px >= rows || py < 0 || py >= cols) { valid = false; break; }
                    if (monsterDist.size()) {
                        if (!(arrive < monsterDist[px][py])) { valid = false; break; }
                    }
                }
                if (!pathCells.empty()) {
                    auto last = pathCells.back();
                    if (!(last.first==rows-1 && last.second==cols-1)) valid = false;
                }
                if (!valid) {
                    pathCells.clear();
                    safePath.clear();
                }
            }
            set<pair<int,int>> playMonsters = monsters;
            vector<vector<int>> md = monsterDist;
            bool playRunning = true; bool paused = false; bool showPath = true;
            int speed_ms = 200;
            int t = 0;
            pair<int,int> ppos = (player.first==-1? find_start() : player);
            size_t escapeStep = 0;
            nodelay(stdscr, TRUE);
            if (!pathCells.empty()) {
                werase(sideWin); box(sideWin,0,0);
                wattron(sideWin, COLOR_PAIR(6) | A_BOLD);
                mvwprintw(sideWin, 4, 2, "Ready to escape!");
                wattroff(sideWin, COLOR_PAIR(6) | A_BOLD);
                mvwprintw(sideWin, 6, 2, "Press SPACE to start");
                wrefresh(sideWin);
                render_tui_play(gridWin, localGrid, md, t, pathCells, ppos, true);   
                nodelay(stdscr, FALSE);
                while (wgetch(stdscr) != ' ');
                nodelay(stdscr, TRUE);
            }
            while (playRunning) {
                werase(sideWin); box(sideWin,0,0);
                mvwprintw(sideWin,1,2,"Playing — space pause, q quit");
                mvwprintw(sideWin,3,2,"Time: %d", t);
                if (safePath.empty()) {
                    wattron(sideWin, COLOR_PAIR(2));
                    mvwprintw(sideWin,4,2,"No escape possible!");
                    wattroff(sideWin, COLOR_PAIR(2));
                } else {
                    wattron(sideWin, COLOR_PAIR(6));
                    mvwprintw(sideWin,4,2,"Path: %zu steps", safePath.size());
                    wattroff(sideWin, COLOR_PAIR(6));
                }
                mvwprintw(sideWin,6,2,"Controls: +/- speed");
                mvwprintw(sideWin,7,2,"          s: toggle path");
                if (!paused && !pathCells.empty() && escapeStep < pathCells.size()) {
                    wattron(sideWin, COLOR_PAIR(6));
                    mvwprintw(sideWin, 9, 2, "Escaping! %zu/%zu", escapeStep+1, pathCells.size());
                    wattroff(sideWin, COLOR_PAIR(6));
                }
                wrefresh(sideWin);

                render_tui_play_dynamic(gridWin, localGrid, playMonsters, t, pathCells, ppos, showPath);

                int k = wgetch(stdscr);
                if (k == ' ') paused = !paused;
                else if (k == 'q') { playRunning = false; break; }
                else if (k == '+') speed_ms = max(10, speed_ms - 20);
                else if (k == '-') speed_ms = min(2000, speed_ms + 20);
                else if (k == 's') showPath = !showPath;
                if (!paused) {
                    playMonsters = spread_monsters(localGrid, playMonsters);
                    t++;
                    if (playMonsters.find(ppos) != playMonsters.end()) {
                        render_tui_play_dynamic(gridWin, localGrid, playMonsters, t, pathCells, ppos, showPath);
                        werase(sideWin); box(sideWin,0,0);
                        wattron(sideWin, COLOR_PAIR(2) | A_BOLD);
                        mvwprintw(sideWin, 4, 2, "CAUGHT BY MONSTERS!");
                        wattroff(sideWin, COLOR_PAIR(2) | A_BOLD);
                        mvwprintw(sideWin, 6, 2, "Press any key");
                        wrefresh(sideWin);
                        napms(1000);
                        nodelay(stdscr, FALSE);
                        wgetch(stdscr);
                        playRunning = false;
                        continue;
                    }
                    if (!pathCells.empty() && escapeStep < pathCells.size()) 
                    {
                        ppos = pathCells[escapeStep];
                        escapeStep++;
                    }
                    if (playMonsters.find(ppos) != playMonsters.end()) {
                        render_tui_play_dynamic(gridWin, localGrid, playMonsters, t, pathCells, ppos, showPath);
                        werase(sideWin); box(sideWin,0,0);
                        wattron(sideWin, COLOR_PAIR(2) | A_BOLD);
                        mvwprintw(sideWin, 4, 2, "CAUGHT BY MONSTERS!");
                        wattroff(sideWin, COLOR_PAIR(2) | A_BOLD);
                        mvwprintw(sideWin, 6, 2, "Press any key");
                        wrefresh(sideWin);
                        napms(1000);
                        nodelay(stdscr, FALSE);
                        wgetch(stdscr);
                        playRunning = false;
                        continue;
                    }
                    if (ppos.first==rows-1 && ppos.second==cols-1) {
                        render_tui_play_dynamic(gridWin, localGrid, playMonsters, t, pathCells, ppos, showPath);
                        werase(sideWin);
                        box(sideWin, 0, 0);
                        wattron(sideWin, COLOR_PAIR(6) | A_BOLD);
                        mvwprintw(sideWin, 4, 2, "ESCAPED!");
                        mvwprintw(sideWin, 6, 2, "Press any key");
                        wattroff(sideWin, COLOR_PAIR(6) | A_BOLD);
                        wrefresh(sideWin);
                        napms(1000);
                        nodelay(stdscr, FALSE);
                        wgetch(stdscr);
                        playRunning = false;
                        continue;
                    }
                    
                    if (t > rows*cols + 50) {
                        playRunning = false;
                    }
                }
                napms(speed_ms);
            }
            nodelay(stdscr, FALSE);
        }
        else if (ch == 'q') { running = false; break; }
    }

    delwin(gridWin); delwin(sideWin);
    endwin();
    return 0;
}
void render_ascii_state(const vector<string>& grid, pair<int,int> playerPos, const set<pair<int,int>>& monsters, int step) {
    cout << "\033[H\033[J";
    cout << "Step: " << step << "\n";
    int rn = grid.size();
    int cm = rn?grid[0].size():0;
    for (int i=0;i<rn;i++){
        for (int j=0;j<cm;j++){
            if (playerPos.first==i && playerPos.second==j) cout << 'A';
            else if (monsters.find({i,j})!=monsters.end()) cout << 'M';
            else cout << grid[i][j];
        }
        cout << '\n';
    }
    cout << "Commands: w/a/s/d to move, q to quit\n";
}
set<pair<int,int>> spread_monsters(const vector<string>& grid, const set<pair<int,int>>& monsters) {
    set<pair<int,int>> next = monsters;
    int n = grid.size();
    int m = n?grid[0].size():0;
    int ddx[4] = {1,-1,0,0};
    int ddy[4] = {0,0,1,-1};
    for (auto p : monsters) {
        for (int d=0;d<4;d++){
            int nx = p.first + ddx[d], ny = p.second + ddy[d];
            if (nx<0||nx>=n||ny<0||ny>=m) continue;
            if (grid[nx][ny] == '#') continue;
            next.insert({nx,ny});
        }
    }
    return next;
}
int cli_game_loop(string infile) {
    int rows = 10, cols = 20;
    vector<string> localGrid;
    set<pair<int,int>> monsters;
    pair<int,int> player = {-1,-1};

    auto init_grid = [&](int r,int c){ rows=r; cols=c; localGrid.assign(r, string(c, '.')); monsters.clear(); player={-1,-1}; };

    if (!infile.empty()) {
        ifstream in(infile);
        if (!in) { cerr << "Failed to open " << infile << "\n"; return 1; }
        read_input_stream(in);
        localGrid = grid;
        rows = n; cols = m;
        monsters.clear(); player={-1,-1};
        for (int i=0;i<rows;i++) for (int j=0;j<cols;j++) {
            if (localGrid[i][j]=='M') monsters.insert({i,j});
            if (localGrid[i][j]=='A') player = {i,j};
        }
    } else {
        init_grid(rows, cols);
    }

    cout << "CLI Game Editor: set up your level. Type 'help' for commands.\n";
    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) return 0;
        if (line.empty()) continue;
        string cmd; vector<string> parts;
        {
            stringstream ss(line);
            while (ss >> cmd) parts.push_back(cmd);
        }
        if (parts.empty()) continue;
        if (parts[0]=="help") {
            cout << "Commands:\n";
            cout << "  size R C         - set grid size\n";
            cout << "  wall R C         - toggle wall\n";
            cout << "  m R C            - toggle monster\n";
            cout << "  a R C            - set player\n";
            cout << "  show             - print grid\n";
            cout << "  play             - start game\n";
            cout << "  load FILE        - load file\n";
            cout << "  save FILE        - save file\n";
            cout << "  quit             - exit\n";
            continue;
        } else if (parts[0]=="size" && parts.size()>=3) {
            int r = stoi(parts[1]); int c = stoi(parts[2]); if (r>0 && c>0) init_grid(r,c);
            cout << "Size: " << rows << "x" << cols << "\n";
            continue;
        } else if (parts[0]=="wall" && parts.size()>=3) {
            int r=stoi(parts[1]), c=stoi(parts[2]); if (r>=0&&r<rows&&c>=0&&c<cols) {
                localGrid[r][c] = (localGrid[r][c]=='#') ? '.' : '#';
            }
            continue;
        } else if (parts[0]=="m" && parts.size()>=3) {
            int r=stoi(parts[1]), c=stoi(parts[2]); if (r>=0&&r<rows&&c>=0&&c<cols) {
                if (monsters.find({r,c})!=monsters.end()) monsters.erase({r,c}); else monsters.insert({r,c});
                if (localGrid[r][c] == '#') localGrid[r][c] = '.';
            }
            continue;
        } else if (parts[0]=="a" && parts.size()>=3) {
            int r=stoi(parts[1]), c=stoi(parts[2]); if (r>=0&&r<rows&&c>=0&&c<cols) player={r,c};
            continue;
        } else if (parts[0]=="show") {
            for (int i=0;i<rows;i++){
                for (int j=0;j<cols;j++){
                    if (player.first==i && player.second==j) cout << 'A';
                    else if (monsters.find({i,j})!=monsters.end()) cout << 'M';
                    else cout << localGrid[i][j];
                }
                cout << '\n';
            }
            continue;
        } else if (parts[0]=="load" && parts.size()>=2) {
            ifstream in(parts[1]); if (!in) { cerr<<"Failed to open "<<parts[1]<<"\n"; }
            else { read_input_stream(in); localGrid = grid; rows = n; cols = m; monsters.clear(); player={-1,-1}; for (int i=0;i<rows;i++) for (int j=0;j<cols;j++){ if (localGrid[i][j]=='M') monsters.insert({i,j}); if (localGrid[i][j]=='A') player={i,j}; }}
            continue;
        } else if (parts[0]=="save" && parts.size()>=2) {
            ofstream out(parts[1]); if (!out) { cerr<<"Failed to write "<<parts[1]<<"\n"; }
            else { out<<rows<<" "<<cols<<"\n"; for (int i=0;i<rows;i++) out<<localGrid[i]<<"\n"; out.close(); cout<<"Saved.\n"; }
            continue;
        } else if (parts[0]=="play") {
            if (player.first==-1) { cout<<"Set player start with 'a R C' first.\n"; continue; }
            
            // CORRECTED: Check if start is safe using monster distance
            n = rows; m = cols; grid = localGrid;
            if (player.first != -1) grid[player.first][player.second] = 'A';
            for (auto mp : monsters) grid[mp.first][mp.second] = 'M';
            compute_monster_dist();
            
            if (monsterDist[player.first][player.second] == 0) {
                cout << "Start position unsafe! Monster at or adjacent to start.\n";
                continue;
            }
            
            set<pair<int,int>> mons = monsters;
            int step = 0;
            pair<int,int> ppos = player;
            bool running = true;
            
            while (running) {
                render_ascii_state(localGrid, ppos, mons, step);
                if (mons.find(ppos) != mons.end()) {
                    cout << "You were caught!\n";
                    running = false;
                    break;
                }
                if (ppos.first==rows-1 && ppos.second==cols-1) {
                    cout << "You escaped!\n";
                    running = false;
                    break;
                }
                string mv;
                if (!getline(cin, mv)) return 0;
                
                if (mv.empty()) {
                } else if (mv=="q") {
                    return 0;
                } else if (mv=="w") {
                    int nx=ppos.first-1, ny=ppos.second;
                    if (nx>=0 && localGrid[nx][ny]!='#') ppos={nx,ny};
                } else if (mv=="s") {
                    int nx=ppos.first+1, ny=ppos.second;
                    if (nx<rows && localGrid[nx][ny]!='#') ppos={nx,ny};
                } else if (mv=="a") {
                    int nx=ppos.first, ny=ppos.second-1;
                    if (ny>=0 && localGrid[nx][ny]!='#') ppos={nx,ny};
                } else if (mv=="d") {
                    int nx=ppos.first, ny=ppos.second+1;
                    if (ny<cols && localGrid[nx][ny]!='#') ppos={nx,ny};
                } else if (mv=="help") {
                    cout << "w/a/s/d=move, Enter=wait, q=quit\n";
                    continue;
                }
                mons = spread_monsters(localGrid, mons);
                step++;
                if (mons.find(ppos) != mons.end()) {
                    render_ascii_state(localGrid, ppos, mons, step);
                    cout << "You were caught!\n";
                    running = false;
                    break;
                }
            }
            continue;
        } else if (parts[0]=="quit") {
            break;
        } else {
            cout << "Unknown command. Type 'help'\n";
            continue;
        }
    }
    return 0;
}
using namespace std;
const int INF = 1e9;
int n, m;
vector<string> grid;
vector<vector<int>> monsterDist;
vector<vector<int>> playerDist;
int dx[4] = {1,-1,0,0};
int dy[4] = {0,0,1,-1};
char dch[4] = {'D','U','R','L'};
void read_input_stream(istream &in) {
    in >> n >> m;
    string line;
    grid.clear();
    grid.reserve(n);
    for (int i = 0; i < n; ++i) {
        in >> line;
        if ((int)line.size() != m) {
            if ((int)line.size() < m) line += string(m - line.size(), '.');
            else line = line.substr(0, m);
        }
        grid.push_back(line);
    }
}

pair<int,int> find_start() {
    for (int i=0;i<n;i++) for (int j=0;j<m;j++) if (grid[i][j]=='A') return {i,j};
    return {-1,-1};
}

vector<pair<int,int>> find_monsters() {
    vector<pair<int,int>> out;
    for (int i=0;i<n;i++) for (int j=0;j<m;j++) if (grid[i][j]=='M') out.push_back({i,j});
    return out;
}

void compute_monster_dist() {
    monsterDist.assign(n, vector<int>(m, INF));
    queue<pair<int,int>> q;
    auto mons = find_monsters();
    for (auto &p : mons) {
        monsterDist[p.first][p.second] = 0;
        q.push(p);
    }
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        int cx = cur.first, cy = cur.second;
        for (int d=0;d<4;d++){
            int nx = cx + dx[d], ny = cy + dy[d];
            if (nx<0||nx>=n||ny<0||ny>=m) continue;
            if (grid[nx][ny] == '#') continue;
            if (monsterDist[nx][ny] > monsterDist[cx][cy] + 1) {
                monsterDist[nx][ny] = monsterDist[cx][cy] + 1;
                q.push({nx,ny});
            }
        }
    }
}

vector<char> find_safe_path() {
    auto start = find_start();
    if (start.first == -1) return {};
    if (monsterDist[start.first][start.second] == 0) return {};
    
    playerDist.assign(n, vector<int>(m, INF));
    vector<vector<pair<int,int>>> parent(n, vector<pair<int,int>>(m, {-1,-1}));
    vector<vector<int>> parentMove(n, vector<int>(m, -1));
    queue<pair<int,int>> q;
    playerDist[start.first][start.second] = 0;
    q.push(start);
    pair<int,int> escape = {-1,-1};
    
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        int cx = cur.first, cy = cur.second;
        pair<int,int> target = {n-1, m-1};
        if (cx == target.first && cy == target.second) {
            escape = cur;
            break;
        }
        
        for (int d=0; d<4; d++){
            int nx = cx + dx[d], ny = cy + dy[d];
            if (nx<0||nx>=n||ny<0||ny>=m) continue;
            if (grid[nx][ny] == '#') continue;
            int arrive = playerDist[cx][cy] + 1;
            if (arrive < monsterDist[nx][ny] && playerDist[nx][ny] == INF) {
                playerDist[nx][ny] = arrive;
                parent[nx][ny] = {cx,cy};
                parentMove[nx][ny] = d;
                q.push({nx,ny});
            }
        }
    }
    
    if (escape.first == -1) return {};
    vector<char> path;
    pair<int,int> cur = escape;
    while (!(cur.first == start.first && cur.second == start.second)) {
        int mv = parentMove[cur.first][cur.second];
        path.push_back(dch[mv]);
        cur = parent[cur.first][cur.second];
    }
    reverse(path.begin(), path.end());
    return path;
}

void render_ncurses(int t, pair<int,int> playerPos, const vector<pair<int,int>>& pathCells, bool showPath) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int offy = 1, offx = 1;
    erase();
    mvprintw(0,0, "PathFinder++ — step: %d  (Space=pause n=step s=path q=quit)", t);
    
    for (int i=0;i<n;i++){
        for (int j=0;j<m;j++){
            int y = offy + i, x = offx + j;
            char ch = grid[i][j];
            
            if (ch == '#') {
                attron(COLOR_PAIR(1)); mvaddch(y,x,'#'); attroff(COLOR_PAIR(1));
            } else if (playerPos.first==i && playerPos.second==j) {
                attron(COLOR_PAIR(3)); mvaddch(y,x,'A'); attroff(COLOR_PAIR(3));
            } else if (grid[i][j] == 'M') {
                attron(COLOR_PAIR(2)); mvaddch(y,x,'M'); attroff(COLOR_PAIR(2));
            } else if (showPath && find(pathCells.begin(), pathCells.end(), make_pair(i,j))!=pathCells.end()) {
                attron(COLOR_PAIR(5)); mvaddch(y,x,'*'); attroff(COLOR_PAIR(5));
            } else if (monsterDist[i][j] <= t) {
                attron(COLOR_PAIR(2)); mvaddch(y,x,'m'); attroff(COLOR_PAIR(2));
            } else {
                mvaddch(y,x,'.');
            }
        }
    }
    refresh();
}

int main(int argc, char** argv) {
    bool playMode = false;
    string infile;
    bool cliGame = false;
    bool tui = false;
    
    for (int i=1;i<argc;i++){
        string a = argv[i];
        if (a == "--play" || a == "--game") playMode = true;
        else if (a == "--cli-game") cliGame = true;
        else if (a == "--tui") tui = true;
        else if ((a=="--input" || a=="-i") && i+1<argc) infile = argv[++i];
        else if (a.rfind("--input=",0)==0) infile = a.substr(8);
    }
    
    if (cliGame) {
        return cli_game_loop(infile);
    }
    if (tui) {
        return tui_game_loop(infile);
    }
    if (infile.empty() && isatty(fileno(stdin))) {
        while (true) {
            cout << "\nPathFinder++ — Menu\n";
            cout << "1) Load maze from file\n";
            cout << "2) Show current maze\n";
            cout << "3) Compute solution\n";
            cout << "4) ASCII animation\n";
            cout << "5) Quit\n";
            cout << "Choose: ";
            string choice;
            if (!getline(cin, choice)) return 0;
            
            if (choice == "1") {
                cout << "File path: ";
                string fp;
                if (!getline(cin, fp)) continue;
                ifstream in(fp);
                if (!in) {
                    cerr << "Failed to open " << fp << "\n";
                    continue;
                }
                read_input_stream(in);
                compute_monster_dist();
                cout << "Loaded " << n << "x" << m << " maze\n";
            } else if (choice == "2") {
                if (grid.empty()) {
                    cout << "No maze loaded.\n";
                    continue;
                }
                cout << "Maze (" << n << "x" << m << "):\n";
                for (int i=0;i<n;i++) cout << grid[i] << '\n';
            } else if (choice == "3") {
                if (grid.empty()) {
                    cout << "No maze loaded.\n";
                    continue;
                }
                compute_monster_dist();
                auto path = find_safe_path();
                if (path.empty()) {
                    cout << "NO\n";
                } else {
                    cout << "YES\n" << path.size() << "\n";
                    for (char c: path) cout << c;
                    cout << "\n";
                }
            } else if (choice == "4") {
                if (grid.empty()) {
                    cout << "No maze loaded.\n";
                    continue;
                }
                compute_monster_dist();
                auto path = find_safe_path();
                vector<pair<int,int>> pathCells;
                
                if (!path.empty()){
                    auto st = find_start();
                    int x = st.first, y = st.second;
                    for (char c: path) {
                        int dir = 0;
                        if (c=='D') dir = 0;
                        else if (c=='U') dir = 1;
                        else if (c=='R') dir = 2;
                        else if (c=='L') dir = 3;
                        x += dx[dir]; y += dy[dir];
                        pathCells.push_back({x,y});
                    }
                }

                auto print_ascii = [&](int t, pair<int,int> playerPos, bool showPath) {
                    cout << "\033[H\033[J";
                    cout << "Step: " << t << "\n";
                    for (int i=0;i<n;i++){
                        for (int j=0;j<m;j++){
                            char ch = grid[i][j];
                            if (i==playerPos.first && j==playerPos.second) cout << 'A';
                            else if (ch == '#') cout << '#';
                            else if (ch == 'M') cout << 'M';
                            else if (showPath && find(pathCells.begin(), pathCells.end(), make_pair(i,j))!=pathCells.end()) cout << '*';
                            else if (monsterDist[i][j] <= t) cout << 'm';
                            else cout << '.';
                        }
                        cout << '\n';
                    }
                };

                bool showPath = true;
                cout << "Controls: Enter=step, a=autoplay, s=toggle path, q=quit\n";
                int t = 0;
                pair<int,int> playerPos = find_start();
                
                if (playerPos.first != -1 && monsterDist[playerPos.first][playerPos.second] == 0) {
                    cout << "Start unsafe! Monster at/adjacent to start.\n";
                    continue;
                }
                
                bool autoplay = false;
                int speed_ms = 200;
                
                while (true) {
                    print_ascii(t, playerPos, showPath);
                    
                    if (!autoplay) {
                        string cmd;
                        if (!getline(cin, cmd)) break;
                        
                        if (cmd == "") {
                            t++;
                            if ((int)pathCells.size() >= t) playerPos = pathCells[t-1];
                        } else if (cmd == "a") {
                            autoplay = true;
                        } else if (cmd == "s") {
                            showPath = !showPath;
                        } else if (cmd == "q") {
                            break;
                        }
                    } else {
                        this_thread::sleep_for(chrono::milliseconds(speed_ms));
                        t++;
                        if ((int)pathCells.size() >= t) playerPos = pathCells[t-1];
                        if (monsterDist[playerPos.first][playerPos.second] <= t) {
                            print_ascii(t, playerPos, showPath);
                            cout << "Caught!\n";
                            break;
                        }
                        
                        if (playerPos.first==n-1 && playerPos.second==m-1) {
                            print_ascii(t, playerPos, showPath);
                            cout << "Escaped!\n";
                            break;
                        }
                        
                        if (cin.rdbuf()->in_avail()>0) {
                            string inp;
                            getline(cin, inp);
                            if (inp == "q") break;
                            if (inp == "") autoplay = false;
                        }
                    }
                }
            } else if (choice == "5") {
                break;
            } else {
                cout << "Unknown option.\n";
            }
        }
        return 0;
    } else {
        if (!infile.empty()) {
            ifstream in(infile);
            if (!in) {
                cerr << "Failed to open " << infile << "\n";
                return 1;
            }
            read_input_stream(in);
        } else {
            read_input_stream(cin);
        }
    }
    
    compute_monster_dist();
    auto path = find_safe_path();
    
    if (path.empty()) {
        cout << "NO\n";
        if (!playMode) return 0;
    } else {
        cout << "YES\n" << path.size() << "\n";
        for (char c: path) cout << c;
        cout << "\n";
    }

    if (!playMode) return 0;
    vector<pair<int,int>> pathCells;
    if (!path.empty()){
        auto start = find_start();
        int x = start.first, y = start.second;
        for (char c: path) {
            int dir = 0;
            if (c=='D') dir = 0;
            else if (c=='U') dir = 1;
            else if (c=='R') dir = 2;
            else if (c=='L') dir = 3;
            x += dx[dir]; y += dy[dir];
            pathCells.push_back({x,y});
        }
    }
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, -1);
        init_pair(2, COLOR_RED, -1);
        init_pair(3, COLOR_CYAN, -1);
        init_pair(5, COLOR_MAGENTA, -1);
    }

    bool paused = true;
    int speed_ms = 200;
    int t = 0;
    auto start = find_start();
    pair<int,int> playerPos = start;
    bool showPath = true;
    render_ncurses(t, playerPos, pathCells, showPath);
    while (true) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch == 'Q') break;
            if (ch == ' ') paused = !paused;
            if (ch == 'n') paused = true;
            if (ch == 's') showPath = !showPath;
            if (ch == '+') speed_ms = max(10, speed_ms - 20);
            if (ch == '-') speed_ms = min(2000, speed_ms + 20);
            
            if (ch == KEY_UP || ch=='w') {
                int nx = playerPos.first-1, ny = playerPos.second;
                if (nx>=0 && grid[nx][ny]!='#') playerPos = {nx,ny};
            }
            if (ch == KEY_DOWN || ch=='s') {
                int nx = playerPos.first+1, ny = playerPos.second;
                if (nx<n && grid[nx][ny]!='#') playerPos = {nx,ny};
            }
            if (ch == KEY_LEFT || ch=='a') {
                int nx = playerPos.first, ny = playerPos.second-1;
                if (ny>=0 && grid[nx][ny]!='#') playerPos = {nx,ny};
            }
            if (ch == KEY_RIGHT || ch=='d') {
                int nx = playerPos.first, ny = playerPos.second+1;
                if (ny<m && grid[nx][ny]!='#') playerPos = {nx,ny};
            }
        }
        if (!paused) {
            t++;
            if (!path.empty() && (int)pathCells.size() >= t) {
                playerPos = pathCells[t-1];
            }
        }   
        render_ncurses(t, playerPos, pathCells, showPath);
        if (monsterDist[playerPos.first][playerPos.second] <= t) {
            mvprintw(n+2, 0, "Caught! Press q to quit.");
            refresh();
            paused = true;
        }
        
        if (playerPos.first==n-1 && playerPos.second==m-1) {
            mvprintw(n+2, 0, "Escaped! Press q to quit.");
            refresh();
            paused = true;
        }
        this_thread::sleep_for(chrono::milliseconds(speed_ms));
    }
    endwin();
    return 0;
}
