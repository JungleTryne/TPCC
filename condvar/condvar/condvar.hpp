#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace solutions {

class ConditionVariable {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable

  template <class Mutex>
  void Wait(Mutex& mutex) {
    /* Идея решения:
     *
     * Начинаем с наивной реализации: atomic будет выполнять
     * роль "очереди", просто создаем атомарную переменную,
     * никогда никак ее не меняем, вызовы Notify[One/All] дублируем.
     *
     * Возникает проблема атомарности блокировки мьютекса, которая
     * указана в условии задачи. Для этого я в Notify[One/All] теперь
     * меняю мою атомарную переменную перед тем, как будить потоки.
     *
     * В таком случае, если между mutex.unlock() и notified.wait(0) у
     * меня влезет какой то Notify из другого потока, то мой текущий
     * поток просто продолжит выполнение, ибо wait не сработает.
     *
     * Казалось бы, если мы изменили notified_, то мы его должны поставить
     * назад в нолик. Это дает TL на стресс тестах -- окей, допустим.
     *
     * Но то, что меня действительно ставит в тупик, так это то, что
     * если не выставлять значение notified_ в ноль, то все тесты проходит,
     * и юнит, и стресс! В чем тогда смысл жизни? =(
     */

    mutex.unlock();

    notified_.wait(0);

    mutex.lock();

    // notified_.store(0); <-- TL
  }

  void NotifyOne() {
    notified_.store(1);
    notified_.notify_one();
  }

  void NotifyAll() {
    notified_.store(1);
    notified_.notify_all();
  }

 private:
  twist::stdlike::atomic<uint32_t> notified_{0};
};

}  // namespace solutions
