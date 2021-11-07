# CAO-Assessment2
Github repo: https://github.com/SimonMatz/CAO-Assessment2

This program was tested on an eight core CPU with 16 logical processors.

I wanted to find a way to automatically divide the amount of selected pictures evenly between the available logical processors,
when the number of pictures selected exceeds the amount of processors available (16 in my case).

Ideally when you select for example 50 images, 14 threads should load 3 pictures each and the other 2 threads 4 pictures.
So you can load all 50 images in parallel with the best amount of threads.

I struggled to implement this idea though. The closest I could get is: If the amount of images is greater than the logical processors available,
assign two pictures per thread. In my case this works great for up to 32 images (16 threads x 2 images). But anything above will create more threads
than available logical processors so the performance decreases.

It also not an ideal solution because when you select for example 17 images, it will create 8 threads with 2 images each plus 1 thread with 1 image.
So 5 possible extra threads are not being utilised.

The fact that I had to use so many if and for loops also made to program slower, but this was the only way to implement this type of thread management without
using the threadpool.

Known bugs/errors:
- When you drag the window to make it smaller/bigger after the images are painted, images are drawn again and fill the whole screen
- The timer only works on the first run. When you go to selected a second batch of images, the timer doesn't reset and it paints on top
- When you select more than 16 images in the first run, the second time you select images and load again it doesn't load/paint all images correctly.
