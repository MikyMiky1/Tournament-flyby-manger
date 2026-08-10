#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <map>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

char getHiddenChar() {
#ifdef _WIN32
    return _getch();
#else
    termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

class Player {
public:
    std::string name;
    Player(const std::string& n = "") : name(n) {}
};

class Team {
public:
    std::string name;
    std::string icon;
    int wins;
    std::vector<Player> players;

    Team(const std::string& n = "", const std::string& ic = "")
        : name(n), icon(ic), wins(0) {}

    std::string getPlayersStr() const {
        std::string s;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i > 0) s += ",";
            s += players[i].name;
        }
        return s;
    }
};

enum MatchType { TOURNAMENT, QUICK_1v1, QUICK_1vAI };

struct MatchHistory {
    MatchType type;
    std::string round;
    std::string side1;
    std::string side2;
    std::string winner;
    int score1;
    int score2;

    MatchHistory(MatchType t, const std::string& r, const std::string& s1,
        const std::string& s2, const std::string& w, int sc1 = 0, int sc2 = 0)
        : type(t), round(r), side1(s1), side2(s2), winner(w), score1(sc1), score2(sc2) {}
};

class FilePersistence {
public:
    void saveTeams(const std::vector<Team>& teams) {
        std::ofstream out("teams.txt");
        if (!out) return;
        for (const auto& t : teams) {
            out << t.name << "|" << t.icon << "|" << t.wins << "|" << t.getPlayersStr() << "\n";
        }
    }

    void loadTeams(std::vector<Team>& teams) {
        std::ifstream in("teams.txt");
        if (!in) return;
        teams.clear();
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string name, icon, winsStr, playersStr;
            if (std::getline(iss, name, '|') &&
                std::getline(iss, icon, '|') &&
                std::getline(iss, winsStr, '|') &&
                std::getline(iss, playersStr)) {
                Team team(name, icon);
                team.wins = std::stoi(winsStr);
                std::istringstream pss(playersStr);
                std::string playerName;
                while (std::getline(pss, playerName, ',')) {
                    if (!playerName.empty())
                        team.players.emplace_back(playerName);
                }
                teams.push_back(team);
            }
        }
    }

    void saveHistory(const std::vector<MatchHistory>& history) {
        std::ofstream out("history.txt");
        if (!out) return;
        for (const auto& m : history) {
            out << static_cast<int>(m.type) << "|" << m.round << "|"
                << m.side1 << "|" << m.side2 << "|" << m.winner << "|"
                << m.score1 << "|" << m.score2 << "\n";
        }
    }

    void loadHistory(std::vector<MatchHistory>& history) {
        std::ifstream in("history.txt");
        if (!in) return;
        history.clear();
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string typeStr, round, side1, side2, winner, sc1, sc2;
            if (std::getline(iss, typeStr, '|') &&
                std::getline(iss, round, '|') &&
                std::getline(iss, side1, '|') &&
                std::getline(iss, side2, '|') &&
                std::getline(iss, winner, '|') &&
                std::getline(iss, sc1, '|') &&
                std::getline(iss, sc2)) {
                MatchType type = static_cast<MatchType>(std::stoi(typeStr));
                history.emplace_back(type, round, side1, side2, winner,
                    std::stoi(sc1), std::stoi(sc2));
            }
        }
    }

    void saveSettings(const std::string& inputMode) {
        std::ofstream out("settings.txt");
        if (!out) return;
        out << "input_mode=" << inputMode << "\n";
    }

    void loadSettings(std::string& inputMode) {
        std::ifstream in("settings.txt");
        if (!in) return;
        std::string line;
        if (std::getline(in, line)) {
            if (line == "input_mode=hidden") inputMode = "hidden";
            else inputMode = "clear";
        }
    }

    struct TournamentArchive {
        std::string date;
        std::string champion;
        std::vector<MatchHistory> matches;
    };

    void saveArchive(const std::vector<TournamentArchive>& archive) {
        std::ofstream out("archive.txt");
        if (!out) return;
        for (const auto& arc : archive) {
            out << "DATE: " << arc.date << "\n";
            out << "CHAMPION: " << arc.champion << "\n";
            for (const auto& m : arc.matches) {
                out << static_cast<int>(m.type) << "|" << m.round << "|"
                    << m.side1 << "|" << m.side2 << "|" << m.winner << "|"
                    << m.score1 << "|" << m.score2 << "\n";
            }
            out << "\n";
        }
    }

    void loadArchive(std::vector<TournamentArchive>& archive) {
        std::ifstream in("archive.txt");
        if (!in) return;
        archive.clear();
        std::string line;
        TournamentArchive current;
        bool inTournament = false;
        while (std::getline(in, line)) {
            if (line.empty()) {
                if (inTournament) {
                    archive.push_back(current);
                    current = TournamentArchive();
                    inTournament = false;
                }
                continue;
            }
            if (line.rfind("DATE: ", 0) == 0) {
                current.date = line.substr(6);
                inTournament = true;
            } else if (line.rfind("CHAMPION: ", 0) == 0) {
                current.champion = line.substr(10);
            } else {
                std::istringstream iss(line);
                std::string typeStr, round, side1, side2, winner, sc1, sc2;
                if (std::getline(iss, typeStr, '|') &&
                    std::getline(iss, round, '|') &&
                    std::getline(iss, side1, '|') &&
                    std::getline(iss, side2, '|') &&
                    std::getline(iss, winner, '|') &&
                    std::getline(iss, sc1, '|') &&
                    std::getline(iss, sc2)) {
                    MatchType type = static_cast<MatchType>(std::stoi(typeStr));
                    current.matches.emplace_back(type, round, side1, side2, winner,
                        std::stoi(sc1), std::stoi(sc2));
                }
            }
        }
        if (inTournament) archive.push_back(current);
    }
};

class MoveHandler {
    bool hiddenMode;
public:
    MoveHandler() : hiddenMode(true) {}

    void setMode(bool hidden) { hiddenMode = hidden; }
    bool isHidden() const { return hiddenMode; }

    char getMove(const std::string& playerName) {
        while (true) {
            std::cout << playerName << ", enter your move (R/P/S): ";
            char move;
            if (hiddenMode) {
                move = getHiddenChar();
                move = toupper(move);
                if (move == 'R' || move == 'P' || move == 'S') {
                    std::cout << "\n";
                    return move;
                }
                std::cout << "\nInvalid move. Use R, P, or S.\n";
            } else {
                std::cin >> move;
                move = toupper(move);
                if (move == 'R' || move == 'P' || move == 'S') {
                    std::cin.ignore(1000, '\n');
                    return move;
                }
                std::cout << "Invalid move. Use R, P, or S.\n";
                std::cin.clear();
                std::cin.ignore(1000, '\n');
            }
        }
    }
};

class RandomMoveGenerator {
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
public:
    RandomMoveGenerator() : gen(std::random_device{}()), dis(0, 2) {}
    char getRandomMove() {
        const char moves[] = { 'R', 'P', 'S' };
        return moves[dis(gen)];
    }
};

struct MatchResult {
    std::string winner;
    int score1, score2;
};

class Match {
public:
    virtual ~Match() {}
    virtual MatchResult play() = 0;
    virtual std::string getRoundDescription() const = 0;
    virtual std::string getSide1() const = 0;
    virtual std::string getSide2() const = 0;
    virtual MatchType getType() const = 0;
    virtual bool isByeMatch() const { return false; }
};

class QuickMatch : public Match {
    std::string player1, player2;
    bool isAI;
    MoveHandler& moveHandler;
    RandomMoveGenerator aiGen;
public:
    QuickMatch(const std::string& p1, const std::string& p2, bool ai, MoveHandler& mh)
        : player1(p1), player2(p2), isAI(ai), moveHandler(mh) {}

    MatchResult play() override {
        int score[2] = { 0, 0 };
        int roundNum = 1;
        std::cout << BOLD << "\n--- " << player1 << " vs " << player2 << " ---\n" << RESET;
        while (score[0] < 2 && score[1] < 2) {
            std::cout << "\nRound " << roundNum << ":\n";
            char move1 = moveHandler.getMove(player1);
            char move2 = isAI ? aiGen.getRandomMove() : moveHandler.getMove(player2);
            if (isAI) std::cout << player2 << " (AI) chose: " << move2 << "\n";
            int result = compareMoves(move1, move2);
            if (result == 0) std::cout << "Tie!\n";
            else if (result == 1) {
                std::cout << player1 << " wins the round.\n";
                score[0]++;
            } else {
                std::cout << player2 << " wins the round.\n";
                score[1]++;
            }
            std::cout << "Score: " << score[0] << " - " << score[1] << "\n";
            roundNum++;
        }
        std::string winnerName = (score[0] == 2) ? player1 : player2;
        std::cout << BOLD << GREEN << "\nMATCH WINNER: " << winnerName << RESET << "\n\n";
        return { winnerName, score[0], score[1] };
    }

    std::string getRoundDescription() const override { return ""; }
    std::string getSide1() const override { return player1; }
    std::string getSide2() const override { return player2; }
    MatchType getType() const override { return isAI ? QUICK_1vAI : QUICK_1v1; }

private:
    static int compareMoves(char m1, char m2) {
        if (m1 == m2) return 0;
        if ((m1 == 'R' && m2 == 'S') || (m1 == 'S' && m2 == 'P') || (m1 == 'P' && m2 == 'R')) return 1;
        return 2;
    }
};

class TournamentMatch : public Match {
    std::string teamA, teamB;
    std::string playerA, playerB;
    bool bye;
    std::string roundDesc;
    MoveHandler& moveHandler;
    RandomMoveGenerator aiGen;
public:
    TournamentMatch(const std::string& ta, const std::string& tb,
        const std::string& pa, const std::string& pb,
        bool isBye, MoveHandler& mh)
        : teamA(ta), teamB(tb), playerA(pa), playerB(pb),
          bye(isBye), moveHandler(mh) {}

    void setRoundDescription(const std::string& desc) { roundDesc = desc; }

    MatchResult play() override {
        if (bye) {
            MatchResult res;
            res.winner = (teamA == "BYE") ? teamB : teamA;
            res.score1 = (teamA == "BYE") ? 0 : 2;
            res.score2 = (teamB == "BYE") ? 0 : 2;
            std::cout << teamA << " vs " << teamB << " -> " << res.winner << " advances (BYE)\n";
            return res;
        }
        int score[2] = { 0, 0 };
        int roundNum = 1;
        std::cout << BOLD << "\n--- " << playerA << " (" << teamA << ") vs "
                  << playerB << " (" << teamB << ") ---\n" << RESET;
        while (score[0] < 2 && score[1] < 2) {
            std::cout << "\nRound " << roundNum << ":\n";
            char move1 = moveHandler.getMove(playerA + " (" + teamA + ")");
            char move2 = moveHandler.getMove(playerB + " (" + teamB + ")");
            int result = compareMoves(move1, move2);
            if (result == 0) std::cout << "Tie!\n";
            else if (result == 1) {
                std::cout << playerA << " wins the round.\n";
                score[0]++;
            } else {
                std::cout << playerB << " wins the round.\n";
                score[1]++;
            }
            std::cout << "Score: " << score[0] << " - " << score[1] << "\n";
            roundNum++;
        }
        std::string winnerName = (score[0] == 2) ? teamA : teamB;
        std::cout << BOLD << GREEN << "\nMATCH WINNER: " << winnerName << RESET << "\n\n";
        return { winnerName, score[0], score[1] };
    }

    std::string getRoundDescription() const override { return roundDesc; }
    std::string getSide1() const override { return teamA; }
    std::string getSide2() const override { return teamB; }
    MatchType getType() const override { return TOURNAMENT; }
    bool isByeMatch() const override { return bye; }

private:
    static int compareMoves(char m1, char m2) {
        if (m1 == m2) return 0;
        if ((m1 == 'R' && m2 == 'S') || (m1 == 'S' && m2 == 'P') || (m1 == 'P' && m2 == 'R')) return 1;
        return 2;
    }
};

class TeamManager {
private:
    std::vector<Team> teams;
    FilePersistence& persistence;
    static const int MAX_TEAMS = 8;
    static const int MAX_PLAYERS = 5;
public:
    TeamManager(FilePersistence& pers) : persistence(pers) {
        persistence.loadTeams(teams);
    }
    ~TeamManager() { persistence.saveTeams(teams); }

    const std::vector<Team>& getTeams() const { return teams; }
    std::vector<Team>& getTeamsRef() { return teams; }

    bool addTeam(const std::string& name, const std::string& icon) {
        if (teams.size() >= MAX_TEAMS) {
            std::cout << RED << "Maximum number of teams reached (8).\n" << RESET;
            return false;
        }
        for (const auto& t : teams) {
            if (t.name == name) {
                std::cout << RED << "A team with that name already exists.\n" << RESET;
                return false;
            }
        }
        teams.emplace_back(name, icon);
        persistence.saveTeams(teams);
        return true;
    }

    bool removeTeam(const std::string& name) {
        auto it = std::find_if(teams.begin(), teams.end(),
            [&](const Team& t) { return t.name == name; });
        if (it == teams.end()) {
            std::cout << RED << "Team not found.\n" << RESET;
            return false;
        }
        teams.erase(it);
        persistence.saveTeams(teams);
        return true;
    }

    bool addPlayer(const std::string& teamName, const std::string& playerName) {
        for (auto& t : teams) {
            if (t.name == teamName) {
                if (t.players.size() >= MAX_PLAYERS) {
                    std::cout << RED << "Team already has 5 players.\n" << RESET;
                    return false;
                }
                t.players.emplace_back(playerName);
                persistence.saveTeams(teams);
                return true;
            }
        }
        std::cout << RED << "Team not found.\n" << RESET;
        return false;
    }

    int getTeamIndex(const std::string& name) const {
        for (size_t i = 0; i < teams.size(); ++i) {
            if (teams[i].name == name) return i;
        }
        return -1;
    }

    void incrementWins(const std::string& teamName) {
        for (auto& t : teams) {
            if (t.name == teamName) {
                t.wins++;
                persistence.saveTeams(teams);
                return;
            }
        }
    }

    std::vector<Team> getRankings() const {
        std::vector<Team> sorted = teams;
        std::sort(sorted.begin(), sorted.end(),
            [](const Team& a, const Team& b) { return a.wins > b.wins; });
        return sorted;
    }
};

class HistoryManager {
private:
    std::vector<MatchHistory> history;
    FilePersistence& persistence;
public:
    HistoryManager(FilePersistence& pers) : persistence(pers) {
        persistence.loadHistory(history);
    }
    ~HistoryManager() { persistence.saveHistory(history); }

    void addEntry(const MatchHistory& entry) {
        history.push_back(entry);
        persistence.saveHistory(history);
    }

    const std::vector<MatchHistory>& getHistory() const { return history; }

    void displayAll() const {
        if (history.empty()) {
            std::cout << "No match history yet.\n";
            return;
        }
        std::cout << BOLD << "=== MATCH HISTORY ===\n" << RESET;
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            std::cout << "Type: " << (it->type == TOURNAMENT ? "Tournament" :
                (it->type == QUICK_1v1 ? "Quick 1v1" : "Quick 1vAI"));
            if (!it->round.empty()) std::cout << " | " << it->round;
            std::cout << " | " << it->side1 << " vs " << it->side2
                      << " -> Winner: " << it->winner
                      << " (" << it->score1 << "-" << it->score2 << ")\n";
        }
    }
};

class Tournament {
private:
    TeamManager& teamManager;
    HistoryManager& historyManager;
    MoveHandler& moveHandler;
    FilePersistence& persistence;
    std::string champion;
    std::vector<MatchHistory> tournamentMatches;

public:
    Tournament(TeamManager& tm, HistoryManager& hm, MoveHandler& mh, FilePersistence& pers)
        : teamManager(tm), historyManager(hm), moveHandler(mh), persistence(pers) {}

    bool start(const std::vector<Team>& allTeams) {
        std::vector<Team> participants;
        for (const auto& t : allTeams) {
            if (!t.players.empty()) participants.push_back(t);
        }
        if (participants.size() < 2) {
            std::cout << RED << "Need at least 2 teams with at least 1 player each to start a tournament.\n" << RESET;
            return false;
        }
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(participants.begin(), participants.end(), std::default_random_engine(seed));
        int size = 1;
        while (size < (int)participants.size()) size <<= 1;
        std::vector<std::string> bracket;
        for (int i = 0; i < size; ++i) {
            if (i < (int)participants.size()) bracket.push_back(participants[i].name);
            else bracket.push_back("BYE");
        }
        std::cout << BOLD << CYAN << "\n=== TOURNAMENT STARTED ===\n" << RESET;
        int roundNum = 1;
        tournamentMatches.clear();
        while (bracket.size() > 1) {
            std::cout << "\n--- Round " << roundNum << " ---\n";
            std::vector<std::string> winners;
            for (size_t i = 0; i < bracket.size(); i += 2) {
                std::string sideA = bracket[i];
                std::string sideB = bracket[i + 1];
                bool byeMatch = (sideA == "BYE" || sideB == "BYE");
                std::string playerA, playerB;
                if (!byeMatch) {
                    int idxA = teamManager.getTeamIndex(sideA);
                    int idxB = teamManager.getTeamIndex(sideB);
                    playerA = teamManager.getTeams()[idxA].players[0].name;
                    playerB = teamManager.getTeams()[idxB].players[0].name;
                }
                TournamentMatch match(sideA, sideB, playerA, playerB, byeMatch, moveHandler);
                match.setRoundDescription("Round " + std::to_string(roundNum));
                MatchResult res = match.play();
                if (!byeMatch) {
                    teamManager.incrementWins(res.winner);
                }
                tournamentMatches.emplace_back(TOURNAMENT, match.getRoundDescription(),
                    sideA, sideB, res.winner, res.score1, res.score2);
                historyManager.addEntry(tournamentMatches.back());
                winners.push_back(res.winner);
            }
            bracket = winners;
            roundNum++;
        }
        champion = bracket[0];
        std::cout << BOLD << GREEN << "\n=== TOURNAMENT CHAMPION: " << champion << " ===\n" << RESET;
        archiveThisTournament();
        return true;
    }

private:
    void archiveThisTournament() {
        FilePersistence::TournamentArchive arc;
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm_now;
#ifdef _WIN32
        localtime_s(&tm_now, &now);
#else
        localtime_r(&now, &tm_now);
#endif
        char buffer[80];
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M", &tm_now);
        arc.date = buffer;
        arc.champion = champion;
        arc.matches = tournamentMatches;
        std::vector<FilePersistence::TournamentArchive> archive;
        persistence.loadArchive(archive);
        archive.push_back(arc);
        persistence.saveArchive(archive);
    }
};

const std::string FIGHTER1 =
"   ___________\n"
"  /           \\\n"
" |  O     O   |   MICHAEL EMAD\n"
" |     -      |   THE ARCHITECT 25030105\n"
" |  \\_____/   |\n"
"  \\___________/\n"
"    /|     |\\\n"
"   / |     | \\\n"
"      |     |\n"
"     _|     |_\n";

const std::string FIGHTER2 =
"    .-----.\n"
"   /  o o  \\\n"
"  |    ^    |  ZEYAD AYMAN\n"
"  |   '-'   |  THE PHANTOM  25030321\n"
"   \\       /\n"
"    '.---.'\n"
"     |   |\n"
"    /|   |\\\n"
"   / |   | \\\n"
"     _|   |_\n";

const std::string FIGHTER3 =
"   .-------.\n"
"  /   @ @   \\\n"
" |     V     |  AHMAD EID\n"
" |   /   \\   |  THE BEAST 25030069\n"
"  \\  \\___/  /\n"
"   '-------'\n"
"    /|    |\\\n"
"   / |    | \\\n"
"     _|    |_\n";

const std::string FIGHTER4 =
"   .-'''-.\n"
"  /  o o  \\\n"
" |    _    |  OMAR OSAMA MUSTAFA\n"
" |   (_)   |  THE SAGE 25030472\n"
"  \\       /\n"
"   '.___.'\n"
"    /   \\\n"
"   /     \\\n"
"  _|     |_\n";

const std::string FIGHTER5 =
"  .-------.\n"
"  | @   @ |\n"
"  |  / \\  |  KAREEM ASHRAF\n"
"  |  \\_/  |  THE TITAN 25030546\n"
"  '-------'\n"
"   /|   |\\\n"
"  / |   | \\\n"
"    |   |\n"
"    |   |\n"
"   _|   |_\n";

const std::string FIGHTER6 =
"   .===. \n"
"  // o o\\\\\n"
" ((   -  )) ALSAAED MOHAMED MAHMOUD\n"
"  \\\\_=,_//  THE SHADOW  25030508\n"
"   /     \\\n"
"  /       \\\n"
" /         \\\n"
"/           \\\n"
"|     __    |\n"
"|    |  |   |\n"
"|____|__|___|\n";

void showCredits() {
    clearScreen();
    std::cout << CYAN << FIGHTER1 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << MAGENTA << FIGHTER2 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << BLUE << FIGHTER3 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << GREEN << FIGHTER4 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << RED << FIGHTER5 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << WHITE << FIGHTER6 << RESET << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "\nPress ENTER to continue...";
    std::cin.ignore();
    std::cin.get();
}

class Application {
private:
    FilePersistence filePersistence;
    TeamManager teamManager;
    HistoryManager historyManager;
    MoveHandler moveHandler;
    std::string inputMode;
    bool running;

public:
    Application() : teamManager(filePersistence),
        historyManager(filePersistence), running(false) {
        filePersistence.loadSettings(inputMode);
        if (inputMode.empty()) inputMode = "hidden";
        if (inputMode == "hidden") moveHandler.setMode(true);
        else moveHandler.setMode(false);
    }

    void run() {
        showCredits();
        running = true;
        while (running) {
            clearScreen();
            displayMainMenu();
            int choice = getMenuChoice(0, 9);
            processMainChoice(choice);
        }
    }

private:
    void displayMainMenu() const {
        std::cout << BOLD << BLUE << "\n+=====================================+\n";
        std::cout << "         RPS TOURNAMENT MANAGER         \n";
        std::cout << "+=====================================+\n" << RESET;
        std::cout << " 1. View Teams                          \n";
        std::cout << " 2. Create Team                         \n";
        std::cout << " 3. Add Player to Team                  \n";
        std::cout << " 4. Quick Match (1v1 / 1vAI)            \n";
        std::cout << " 5. Start Tournament                    \n";
        std::cout << " 6. Match History                       \n";
        std::cout << " 7. Rankings                            \n";
        std::cout << " 8. Past Tournaments Archive            \n";
        std::cout << " 9. Settings                            \n";
        std::cout << " 0. Exit                                \n";
        std::cout << BOLD << BLUE << "+=====================================+\n" << RESET;
    }

    int getMenuChoice(int min, int max) {
        int choice;
        std::cout << "Enter your choice: ";
        while (!(std::cin >> choice) || choice < min || choice > max) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid. Enter a number between " << min << " and " << max << ": ";
        }
        std::cin.ignore();
        return choice;
    }

    void processMainChoice(int choice) {
        clearScreen();
        switch (choice) {
            case 1: viewTeams(); break;
            case 2: createTeam(); break;
            case 3: addPlayer(); break;
            case 4: quickMatch(); break;
            case 5: startTournament(); break;
            case 6: historyManager.displayAll(); break;
            case 7: showRankings(); break;
            case 8: showArchive(); break;
            case 9: changeSettings(); break;
            case 0: running = false; return;
        }
        if (choice != 0) {
            std::cout << "\nPress ENTER to return to menu...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }

    void viewTeams() const {
        const auto& teams = teamManager.getTeams();
        if (teams.empty()) {
            std::cout << "No teams yet.\n";
            return;
        }
        std::cout << BOLD << "=== TEAMS ===\n" << RESET;
        for (const auto& t : teams) {
            std::cout << t.icon << " " << t.name << "  (Wins: " << t.wins << ")\n";
            std::cout << "   Players: ";
            for (const auto& p : t.players) std::cout << p.name << " ";
            if (t.players.empty()) std::cout << "(none)";
            std::cout << "\n";
        }
    }

    void createTeam() {
        std::cout << "Enter team name: ";
        std::string name;
        std::getline(std::cin, name);
        if (name.empty()) {
            std::cout << "Name cannot be empty.\n";
            return;
        }
        std::cout << "Choose an icon from the list:\n";
        std::cout << "1. Shield  2. Sword  3. Fire  4. Skull  5. Star  6. Spiral\n";
        std::string iconChoice;
        std::getline(std::cin, iconChoice);
        std::string icon;
        if (iconChoice == "1") icon = "[SHIELD]";
        else if (iconChoice == "2") icon = "[SWORD]";
        else if (iconChoice == "3") icon = "[FIRE]";
        else if (iconChoice == "4") icon = "[SKULL]";
        else if (iconChoice == "5") icon = "[STAR]";
        else if (iconChoice == "6") icon = "[SPIRAL]";
        else icon = "[TEAM]";
        if (teamManager.addTeam(name, icon)) {
            std::cout << GREEN << "Team created successfully!\n" << RESET;
        }
    }

    void addPlayer() {
        const auto& teams = teamManager.getTeams();
        if (teams.empty()) {
            std::cout << "No teams. Create one first.\n";
            return;
        }
        std::cout << "Select team:\n";
        for (size_t i = 0; i < teams.size(); ++i)
            std::cout << i + 1 << ". " << teams[i].name << "\n";
        int idx = getMenuChoice(1, teams.size()) - 1;
        std::cout << "Enter player name: ";
        std::string pname;
        std::getline(std::cin, pname);
        if (pname.empty()) {
            std::cout << "Name cannot be empty.\n";
            return;
        }
        if (teamManager.addPlayer(teams[idx].name, pname))
            std::cout << GREEN << "Player added.\n" << RESET;
    }

    void quickMatch() {
        std::cout << "Quick Match Menu:\n1. Human vs Human\n2. Human vs AI\nChoose (1/2): ";
        int sub = getMenuChoice(1, 2);
        std::string player1, player2;
        bool isAI = (sub == 2);
        std::cout << "Enter Player 1 name: ";
        std::getline(std::cin, player1);
        if (isAI) player2 = "AI";
        else {
            std::cout << "Enter Player 2 name: ";
            std::getline(std::cin, player2);
        }
        if (player1.empty() || (!isAI && player2.empty())) {
            std::cout << "Names cannot be empty.\n";
            return;
        }
        QuickMatch match(player1, player2, isAI, moveHandler);
        MatchResult res = match.play();
        historyManager.addEntry(MatchHistory(match.getType(), "", player1, player2,
            res.winner, res.score1, res.score2));
    }

    void startTournament() {
        Tournament t(teamManager, historyManager, moveHandler, filePersistence);
        t.start(teamManager.getTeams());
    }

    void showRankings() const {
        auto ranked = teamManager.getRankings();
        if (ranked.empty()) {
            std::cout << "No teams.\n";
            return;
        }
        std::cout << BOLD << "=== RANKINGS ===\n" << RESET;
        for (size_t i = 0; i < ranked.size(); ++i) {
            std::string medal;
            if (i == 0) medal = "1st";
            else if (i == 1) medal = "2nd";
            else if (i == 2) medal = "3rd";
            else medal = "   ";
            std::cout << medal << " " << ranked[i].icon << " " << ranked[i].name
                      << " - Wins: " << ranked[i].wins << "\n";
        }
    }

    void showArchive() {
        std::vector<FilePersistence::TournamentArchive> archive;
        filePersistence.loadArchive(archive);
        if (archive.empty()) {
            std::cout << "No past tournaments.\n";
            return;
        }
        for (const auto& arc : archive) {
            std::cout << BOLD << "Date: " << arc.date << "  |  Champion: " << arc.champion << RESET << "\n";
            for (const auto& m : arc.matches) {
                std::cout << "  " << m.round << ": " << m.side1 << " vs " << m.side2
                          << " -> " << m.winner << " (" << m.score1 << "-" << m.score2 << ")\n";
            }
            std::cout << "\n";
        }
    }

    void changeSettings() {
        std::cout << "Current input mode: " << inputMode << "\n";
        std::cout << "Change to?\n1. Hidden\n2. Clear-screen\nChoice: ";
        int ch = getMenuChoice(1, 2);
        inputMode = (ch == 1) ? "hidden" : "clear";
        moveHandler.setMode(ch == 1);
        filePersistence.saveSettings(inputMode);
        std::cout << GREEN << "Setting updated.\n" << RESET;
    }
};

int main() {
    Application app;
    app.run();
    return 0;
}