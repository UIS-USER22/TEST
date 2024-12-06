#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <chrono>
#define uint8_t unsigned char

// дан циклический (кольцевой) буфер и некоторые функции работы с ним
#define BUFFER_SIZE 512

#if (BUFFER_SIZE & (BUFFER_SIZE - 1)) != 0
  #error "Incorrect buffer size"
#endif

typedef struct {
  size_t first;
  size_t last;
  uint8_t data[BUFFER_SIZE];
} CircularBuffer;

// ClearBuf очищает буфер (может также использоваться для инициализации структуры CircularBuffer)
void ClearBuf(CircularBuffer* pBuf)
{
  pBuf->first = 0;
  pBuf->last = 0;
}

// ReadByte читает байт из буфера.  если в буфере нет данных, возвращает -1.
int ReadByte(CircularBuffer* pBuf)
{
  if (pBuf->first == pBuf->last)
    return -1;
  int result = pBuf->data[pBuf->first];
  pBuf->first = (pBuf->first + 1) & (BUFFER_SIZE - 1);
  return result;
}

// пишет байт в буфер, возвращает true если запись прошла успешно
bool WriteByte(CircularBuffer* pBuf, uint8_t value)
{
  size_t next = (pBuf->last + 1) & (BUFFER_SIZE - 1);
  if (next == pBuf->first)
    return false;
  pBuf->data[pBuf->last] = value;
  pBuf->last = next;
  return true;
}

// функция IsEmpty возвращает true если буфер пуст, иначе false
// пустым являтся буфер в котором нет данных для чтения.
bool IsEmpty(CircularBuffer* pBuf)
{
  // TODO: напишите код этой фукнции
  return pBuf->first == pBuf->last;
}


// функция IsFull возвращает true если буфер полон, иначе false
// попытка писать в полный буфер всегда будет завершаться неудачей.
bool IsFull(CircularBuffer* pBuf)
{
  // TODO: напишите код этой фукнции
  return ((pBuf->last + 1) & (BUFFER_SIZE - 1)) == pBuf->first;
}

// что возвращает функция GetSomething? переименуйте ее, чтобы название соответствоало возвращаемому значению 
size_t GetSize(CircularBuffer* pBuf)
{
  return (pBuf->last - pBuf->first) & (BUFFER_SIZE - 1); // поверне кількість елементів у буфері
}

// нам нужна фукнция для перемещения данных из одного циклического буфера в другой
// мы решили ее объявить так
// size_t BufMove(CicrularBufffer* pDest, CicrularBufffer* pSource)
// предполагается что pDest и pSource указывают на разные буферы
// функция должна перемещать максимально возможное кол-во байт из Source в Dest
// и возвращать число скопированных байт.
// т.е если в Dest буфере не хватет места для всего содержимого из Source, 
// переместиться должны только те байты для которых есть место, остаток должен остаться в Source буфере 
// если же места хватает, то переместиться должно все, и буфер Source остаться пуст.

// программист написал вот такую фукнцию
// соответствует ли она описаню данному выше?
// какие у нее есть недостатки?
size_t BufMoveSlow(CircularBuffer* pDest, CircularBuffer* pSource)
{
  if (pDest == pSource)
    return 0;
  int value;
  size_t result = 0;
  while ((value = ReadByte(pSource)) != -1 && WriteByte(pDest, value))
    result++;
  return result;
  // ця функція дуже повільна тому що постійно засирає стек визовами ще й двух функцій! Занадто багато дій виходить що визвали умовно там потрібно 3 переміщення 
  //і виходить 3 ReadByte, WriteByte і кожна з них має виконатись по 3 рази. Це без порівнянь і тд
}

// напишите свой вариант функции перемещения данных
size_t BufMoveFast(CircularBuffer* pDest, CircularBuffer* pSource)
{
  // TODO: напишите код этой фукнции
  // фнукция должна соответствовать требованиям описаным в комментарии выше
  // для копирования данных между буферами используйте фукцнию memcpy
  // желательно чтобы число вызовов memcpy не превышало кол-во непрерывных блоков данных, которые нужно скопировать
  // т.е чтобы число вызовов memcpy было минимально возможным.
  if (pDest == pSource)
    return 0;

  size_t srcSize = GetSize(pSource);
  size_t destFree = BUFFER_SIZE - 1 - GetSize(pDest);

  size_t toMove = (srcSize < destFree) ? srcSize : destFree;
  if (toMove == 0)
    return 0;

  size_t srcFirstBlock = BUFFER_SIZE - pSource->first;
  if (srcFirstBlock > toMove)
    srcFirstBlock = toMove;

  size_t destFirstBlock = BUFFER_SIZE - pDest->last;
  if (destFirstBlock > srcFirstBlock)
    destFirstBlock = srcFirstBlock;

  memcpy(pDest->data + pDest->last, pSource->data + pSource->first, destFirstBlock);
  pDest->last = (pDest->last + destFirstBlock) & (BUFFER_SIZE - 1);
  pSource->first = (pSource->first + destFirstBlock) & (BUFFER_SIZE - 1);

  size_t remaining = toMove - destFirstBlock;
  if (remaining > 0) {
    memcpy(pDest->data + pDest->last, pSource->data + pSource->first, remaining);
    pDest->last = (pDest->last + remaining) & (BUFFER_SIZE - 1);
    pSource->first = (pSource->first + remaining) & (BUFFER_SIZE - 1);
  }

  return toMove;
}

// вспомогательная функция для отладки
void PrintBuffer(CircularBuffer* pBuf) 
{ 
  if (pBuf->first == pBuf->last)
    printf(" Empty");
  size_t pos;    
  for (pos = pBuf->first; pos != pBuf->last; pos = (pos + 1) & (BUFFER_SIZE - 1))
    printf(" %02x", pBuf->data[pos]);
  printf("\n");
}

// код ниже можно менять по своему усмотрению для тестирования фукнций

CircularBuffer bufferA;
CircularBuffer bufferB;

int main(){
  ClearBuf(&bufferA);
  ClearBuf(&bufferB);

  // Заповнюємо bufferA
  for (size_t i = 0; i < 256; i++) {
    WriteByte(&bufferA, (uint8_t)(i & 0xFF));
  }

  // Вимірюємо час для BufMoveSlow
  auto startSlow = std::chrono::high_resolution_clock::now();
  size_t resSlow = BufMoveSlow(&bufferB, &bufferA);
  auto endSlow = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diffSlow = endSlow - startSlow;

  printf("BufMoveSlow moved %zu item(s) from BufferA to BufferB\n", resSlow);
  printf("BufMoveSlow time: %.9f seconds\n", diffSlow.count());

  // Перезаповнюємо bufferA
  ClearBuf(&bufferA);
  for (size_t i = 0; i < 256; i++) {
    WriteByte(&bufferA, (uint8_t)(i & 0xFF));
  }
  ClearBuf(&bufferB);

  // Вимірюємо час для BufMoveFast
  auto startFast = std::chrono::high_resolution_clock::now();
  size_t resFast = BufMoveFast(&bufferB, &bufferA);
  auto endFast = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diffFast = endFast - startFast;

  printf("BufMoveFast moved %zu item(s) from BufferA to BufferB\n", resFast);
  printf("BufMoveFast time: %.9f seconds\n", diffFast.count());

  return 0;
}