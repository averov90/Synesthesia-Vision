# Synesthesia Vision
[![License](https://img.shields.io/badge/LICENSE-MIT-green?style=flat-square)](/LICENSE)  [![Version](https://img.shields.io/badge/VERSION-DEBUG%20--%20demonstrable-yellow?style=flat-square)](https://github.com/averov90/Synesthesia-Vision/releases)
### :small_orange_diamond: [English version](/README-eng.md)

###### Если вы не знаете, что такое синестезия, пройдите по [ссылке](https://ru.wikipedia.org/wiki/%D0%A1%D0%B8%D0%BD%D0%B5%D1%81%D1%82%D0%B5%D0%B7%D0%B8%D1%8F#%D0%A5%D1%80%D0%BE%D0%BC%D0%B5%D1%81%D1%82%D0%B5%D0%B7%D0%B8%D1%8F_(%D1%84%D0%BE%D0%BD%D0%BE%D0%BF%D1%81%D0%B8%D1%8F)).

Данная программа представляет поступающую с камеры картинку в звук, точнее, в звук превращается один из каналов изображения: цвет или яркость.

## Синестезия
Программа делает снимок, затем изменяет его размер до 67x50. Картинка такого размера озвучивается по столбцам с помощью изменения интенсивности какой-либо частоты в выходном звуке.

Алгоритм довольно прост:
1. Уменьшить изображение до 67x50
2. Взять из изображения один столбец (1x50)
3. Каждому пикселю в этом столбце сопоставить частоту (напрмер, 8000 Гц для 1 пикселя, 16000 Гц для 2 пикселя и т.д.)
4. Умножить синусоиду соответствующей частоты на коэффициент (от 0 до 1, яркость пикселя 0 - коэффициент = 0, яркость 255 - коэффициент = 1)
5. Сложить умноженные на коэффициент синусоиды.

С помощью синестезии можно научиться "видеть" ушами т.к. изображение начинает звучать. Даже неподготовленный человек за небольшое время сможет отличить наслух (с завязанными глазами) где находится свет, какой он формы, какой он яркости. Если тренероваться больше, то можно научиться "видеть" ч/б картинку через уши. 
*Через 1 канал синестезии можно передать не более, чем 3-мерный вектор: X, Y, B - позиция пикселя по X, Y, и какое-то значение в пикселе. Развёртка, как нетрудно догадаться следующая: 
Y представляется частотой, B - интенсивностью частоты при миксовании, X представляется временем проигрывания столбца (все столбцы проигрываются по очереди циклически с лева на право).*

#### Файлы
- [synesthesia.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/synesthesia.h)
- [synesthesia.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/synesthesia.cpp)

Блок синестезии требует присутствия блока **wavutils**. Другие блоки проекта данному блоку не требуются. 
Функция синестезии была написана под библиотеку **portaudio**. В блоке так же используются классы из **opencv**, но их нетрудно заменить аналогами.

## WavUtils
Данный блок содержит функции для работы со звуковыми файлами, в часности - *wave*. Здесь содержатся класс для получений волны из *.wav* и дальнейшего её использования. 
Так же в этом блоке вводится специальный формат хранения звука - **SND**, а так же средства единичной и пакетной генерации файлов частот (которые используются в блоке синестезии).

#### Файлы
- [wavutils.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavutils.h)
- [wavutils.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavutils.cpp)

Название пространства имён - *WavUtils*.

**ВНИМАНИЕ**: под *частотой* в данном контексте понимается массив абсолютных амплитудных значений.

#### Структуры:
- **WAV_HEADER** - шаблон заголовка *wave*-файла, содержит его параматры (частота дескретизации, каналы и др.)
- **WaveSmallInfo** - структура, возвращаемая в дополнение (опционально) функцией LoadWAVFromBuffer. Данная структура содержит информацию о количестве каналов и частоте дескретизации .wav файла.
- **Freqs** - структура, возвращаемая функциями *GetPeriodicalSineFreqs* и *GetSegmetSineFreqs*. Содержит массив частот в заданном диапазоне, которые можо сгенерировать при заданных параметрах с поморщью функций *GenPeriodicalSine* и *GenSegmentSine* соостветственно. Данная структура поддерживает operator[] и имеет функцию автоматического удаления.
- **SegmentSineProps** - структура, необходимая для генерации частоты функцией *GetSegmetSineFreqs*. Содержит размер буфера частоты и период для генерации. Из этой структуры вам понадобится взять значение размера буфера.
- **SynGenData** - структура, содержащая в себе множество частот. Её использование удобно в случае, когда нужно подгрузить множество файлов за раз (например, 50 файлов частот).

#### Функции:
- **GetPeriodicalSineFreqs** - позволяет получить список частот, поддерживаемых в режиме "Периодический", т.е. частоты, кратные частоте дискретизации. "Периодический" режим явзяется частным случаем "Сегментного" режима. Агрумент *count* - количество частот, которое вы хотите получить, *offset* - смещение в массиве поддерживаемых частот. Например, чтобы получить частоты с 4-й по 9-ю, надо *count*=5, *offset*=4.
- **GetPeriodicalSineBuffLen** - получает необходимый для хранения частоты размер буфера short. В аргументе указывается частота из списка поддерживаемых частот.
- **GenPeriodicalSine** - генерирует частоту. *buffer* - указатель на буфер нужного размера, *lenght* - размер буфера, *tone* - частота (1/T, где T - период), *volume* максимальное абсолютное значение в массиве абсолютных амплитудных значений.
- **GetSegmetSineFreqs** - позволяет получить список частот для функции *GenSegmentSine*. *min_freq* - начальная частота (800 Гц, например), *max_freq* - конечная частота (1600 Гц, например), *accuracy* - степень округления при генерации (0 - генерация, эквивалентная "Periodical", 4 - генерация с максимально возможным для *GetSegmetSine* количеством частот). Если проще - *accuracy* отвечает за количество частот между *min_freq* и *max_freq*. *buffmultiplicity* - максимальное число повторений периода. Для "Periodical" всегда достаточно 1 потому, что "Periodical" всегда является делителем частоты без остатка, а, значит, массив его значений для правильного воспроизведения может содержать всего одно максимальное и одно минимальное значение (один период). Для любой частоты, кроме "Periodical" будет недостаточно одного повторения. Как можно догадаться, *buffmultiplicity* для конкретной частоты нужен такой, чтобы период частоты *buffmultiplicity* был делителем частоты дескретизации. Однако, конкретно моментом рассчёта вам заморачиваться не стоит т.к. для какой-то отдельной частоты может получится слишком большой буфер. Лучше выберите максимальное допустимое число повторений, чтоб не превысить одним буфером гигабайт :)
- **GetSegmetSineBuffLen** - получает структуру *SegmentSineProps*, *accuracy* необходимо указать для рассчёта периода, иначе заданная частота была бы округлена до "Periodical"-соответствующей.
- **GenSegmentSine** - генерирует частоту с использованием *SegmentSineProps*. 
- **LoadWAVFromBuffer** - получает частоту из .wav определённого вида (не float-point, например). *buffer* - указатель на буфер  с wave-файлом. *outbufflen* - возвращает размер созданного буфера для хранения частоты. Дополнительно можно передать указатель на *WaveSmallInfo*, если нужно. Функция возвращает частоту (массив абсолютных амплитудных значений).
- **SaveSND** - сохраняет частоту в .snd файл (меньше, чем .wav). *filepath* - путь для сохранение файла, *buffer* - частота, *bufflen* - длинна частоты (массива абсолютных амплитудных значений). Возращает *true* в случае успеха.
- **LoadSND** - загружает частоту из файла. Возвращает частоту, а через аргумент *bufflen* - размер массива.
- **GenerateSinesByFreqFile** - генерирует множество частот (например, 50), используя файл генерации (текстовый, ASCII). Первая строка - количество частот, остальные строки - частоты (F = 1/T).
<details>
<summary>Пример содержимого файла частот</summary>
  
```
3  
879.89  
659  
155.7 
```
</details>

- **SaveSNDPacket** - сохраняет множество частот в папку *SNDfileFolder*, а так же сохраняет текстовый файл с перечислением файлов частот в папке *SNDfileFolder*. 
<details>
<summary>Пример содержимого файла перечисления файлов с частотами</summary>
  
```
3
aug_0.snd 118
aug_1.snd 626
aug_2.snd 662
```
aug_0.snd 118 - "aug_0.snd" имя файла, 118 - размер данных файла в short. Размер в байтах тут равен = 118 * 2 + 4 (4 - размер uint в начале).
</details>

- **LoadSNDPacket** - загружает множество файлов.

## WavPlayer
Позволяет воспроизвести wave файл синхронно или асинхронно. Вызов из нескольких потоков и запуск асинхронного воспроизведения нескольких файлов работают корректно. Для работы данного блока требуется блок *WavUtils*.

#### Файлы
- [wavplayer.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavplayer.h)
- [wavplayer.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavplayer.cpp)

#### Функции:
- **WavePlayWaitD** - запускает синхронное (ждёт завершение воспроизведения) воспроизведение wave-файла, находящегося в буфере *WAV_file*.
- **WavePlayNowaitD** - запускает асинхронное (не ждёт завершения воспроизведения) воспроизведение wave-файла.
- **SNDPlayWaitD** - запускает синхронное воспроизведение snd-файла.
- **SNDPlayNowaitD** - запускает асинхронное воспроизведение snd-файла.
<details>
<summary>Что значит "D" в конце имени функции?</summary>
"D" в конце имени функции (от англ. "defined") означает, что данная функция использует заранее заданные параметры wave-файла при воспроизведении (sampleRate, bitDepth). Это удобно, когда воспроизводятся wave-файлы только определённого формата.
</details>

## CloudAPI
Позволяет выполнять запросы к *Google-Vision-API* и к *Google-TTS-API*. 

#### Функции:
- **Cloud_TTS** - функция, использующая *Google-TTS-API*. На вход подаётся строка для озвучивания. В случае успешного окончания работы функция вернёт *true* и заменит содержимое аргумента *text* на байты wave-файла, пришедшего из облака.
- **Cloud_ImageDescriptor** - функция, использующая *Google-Vision-API*. На вход подаётся изображение (*image*), в котором нужно найти объекты. Второй аргумент содержит строку, в которую в случае успешного выполнения (возвращено значение *true*) будуть помещены 1-10 тегов, описывающих изображение (напрмер, ```sofa, armchair, cotton```). Теги расположены в порядке убывание их "релевантности". Из функции возвращается строка вида ```Found: tag1, tag2, tag3```. При необходимости слово "found" можно убрать в исходном коде функции.  
Так же в этой функции используется функция создания jpeg-изображения в ram с помощью функции из **libjpeg-turbo** взамен неоптимальному *imdecode* из **opencv**.

*Больше функций Google-TTS-API и Google-Vision-API представлено в проекте, на который есть ссылка ниже.*

### *Существует проект, созданный на основе этого: [Raspberry Pi - SyneVis_Proj](https://github.com/averov90/RaspberryPi-SyneVis_Proj).*
