// void GameManager::handleScriptUpload(const std::string& content) {
//     GameVisitor visitor;
//     auto moves = visitor.parseMoves(content);
    
//     for (const auto& move : moves) {
//         // Execute move
//         board.makeMove(move);
        
//         // Notify all connected clients (including uploader)
//         broadcastGameState();
        
//         std::this_thread::sleep_for(500ms);  // Delay between moves
//     }
// }