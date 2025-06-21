#include <iostream>
#include <memory>
#include <string>

struct User {
  std::string name;
  int age;

  User(const std::string& name, int age) : name(name), age(age) {}

  void greet() {
    std::cout << "HELLO!\n";
  }

  ~User() {
    std::cout << "DESCTRUCTOR\n";
  }
};

void create() {
  std::unique_ptr<User> u(new User("Jordi", 25));
  u->greet();
}

int main() {
  create();
  std::cout << "BYE\n";
}
