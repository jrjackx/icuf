# icuf
Short for "I coded you flowers." small terminal based toy program. Made for my girlfriend.

This is based off of the code I swiped from https://viewsourcecode.org/snaptoken/kilo/. You can see my thought process parsing this information inside of https://github.com/jrjackx/my_kilo . Most of what I learned to create this program is present in the readme of that program's github repo.

## Features:
Every time the program runs, there are a random number of flowers which populate the terminal window. The exact range depends on the window dimensions, but it is usually between 3 - 5 flowers. The flowers begin growing to random heights. Try changing the window size to be wide and flat, or skinny and tall, to see how the flowers react. (CTRL + and CTRL - to change the font size as well.)

AI was used very sparingly for this project. The only AI generated code used is in the implementation of the flower head spawning when the stem has finished growing. I scrutinized each line of AI generated code to make sure I understood it and replicated it myself by hand instead of copy-pasting.
