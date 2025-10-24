#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <stdexcept>

using namespace std;

string  vulner[4] = { "0", "N", "E", "B" };

// A helper function to split a string by a delimiter.
// This is useful for separating the hands and the suits.
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief Converts a single hand from PBN's dot-separated format to LIN's suit-prefixed format.
 * @param pbnHand A string representing one hand, e.g., "QJ9875.K74.96.74".
 * @return A string in LIN format, e.g., "SQJ9875HK74D96C74".
 */
std::string convertHandToLinFormat(const std::string& pbnHand) {
    std::vector<std::string> suits = split(pbnHand, '.');
    if (suits.size() != 4) {
        // Handle cases where a hand might be malformed.
        return "InvalidHand";
    }

    std::stringstream linHand;
    linHand << "S" << suits[0] << "H" << suits[1] << "D" << suits[2] << "C" << suits[3];
    return linHand.str();
}

/**
 * @brief Converts a full PBN deal string to a BBO-compatible LIN format string.
 * @param pbnString The full PBN string, e.g., "N:QJ9875.K74.96.74 62.JT92.AJ43.AT5..."
 * @return The converted LIN string ready for BBO.
 */
std::string pbnToLin(const std::string& pbnString, int d, int vul, int index) {
    // 1. Find the dealer and separate it from the hands
    size_t colonPos = pbnString.find(':');
    if (colonPos == std::string::npos || colonPos == 0) {
        throw std::invalid_argument("Invalid PBN format: Missing or misplaced dealer info.");
    }

    char dealer = 'S';
    std::string allHandsStr = pbnString.substr(colonPos + 1);

    // 2. Split the string into four separate hands
    std::vector<std::string> pbnHands = split(allHandsStr, ' ');
    // Remove any empty strings that might result from multiple spaces
    pbnHands.erase(std::remove_if(pbnHands.begin(), pbnHands.end(), [](const std::string& s) {
        return s.empty();
        }), pbnHands.end());

    if (pbnHands.size() != 4) {
        throw std::invalid_argument("Invalid PBN format: Must contain exactly four hands.");
    }

    // 3. Map PBN hands to players (N, E, S, W) based on the dealer
    std::map<char, std::string> playerHands;
    const std::string players = "NESW";

    for (int i = 0; i < 4; ++i) {
        char currentPlayer = players[i % 4];
        playerHands[currentPlayer] = pbnHands[i];
    }

    // 4. Map the PBN dealer to the LIN dealer number (S=1, W=2, N=3, E=4)
    std::string linDealerNum;
    switch (dealer) {
    case 'S': linDealerNum = "1"; break;
    case 'W': linDealerNum = "2"; break;
    case 'N': linDealerNum = "3"; break;
    case 'E': linDealerNum = "4"; break;
    default:  linDealerNum = "1"; break; // Default to South
    }

    // 5. Build the final LIN string in the required S, W, N, E order
    std::stringstream linString;
    
    linString << "qx|o" << to_string(index) << "|md|" << linDealerNum; // LIN prefix and dealer

    // LIN format requires hands in the order: South, West, North, East
    linString << convertHandToLinFormat(playerHands['S']) << ",";
    linString << convertHandToLinFormat(playerHands['W']) << ",";
    linString << convertHandToLinFormat(playerHands['N']) << ",";
    linString << convertHandToLinFormat(playerHands['E']);

    linString << "|rh||ah|Board " << index << "|sv|" << vulner[vul] << "|pg||"; // LIN suffix

    return linString.str();
}

int main() {
    // Example PBN string
    std::string pbnDeal = "N:QJ9875.K74.96.74 62.JT92.AJ43.AT5 AKT.AQ63.T7.KJ82 43.85.KQ852.Q963";

    try {
        std::string linDeal = pbnToLin(pbnDeal, 1, 1, 2);

        std::cout << "Original PBN String:\n" << pbnDeal << std::endl;
        std::cout << "\nConverted BBO LIN String:\n" << linDeal << std::endl;

    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
