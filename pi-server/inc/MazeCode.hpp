#ifndef MAZE_CODE_HPP
#define MAZE_CODE_HPP

#include <functional>
#include <string>

class MazeCode {
   public:
    using WordProvider = std::function<std::string()>;

    explicit MazeCode(WordProvider provider);

    void reset();
    [[nodiscard]] bool verify(const std::string& code) const;
    [[nodiscard]] std::string value() const;

   private:
    WordProvider provider_;
    std::string code_;
};

#endif  // MAZE_CODE_HPP
