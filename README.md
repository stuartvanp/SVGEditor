This project contains the backend for an SVG image editor and a modest front end

The back end of the program, approximately 2000 lines,  was written in C. In short, it works by using the library Libxml2 to parse an SVG file(they are XML based files) into a Libxml tree. From there my code parses the tree into a struct which contains 4 linked lists to store groups, rectangles circles and paths, and within said groups there can also be more groups, rectangles, circles and paths. This process required lots of recursion as it is possible to have groups contained within groups contained within groups, etc, in a single SVG file. To actually edit the image the struct that my code built is edited with functions that can navigate and modify the struct. Then to save the file my code transforms the struct back into a Libxml tree, and then Libxml2 is used to create a working SVG file from this tree.

The front of the code was built using Node.js to connect the server and the client. My university's servers were used to host the website so I have to be actively working on the site for it to be running. However I have attached screenshots to show you my work! As of right now the site allows for upload and download of SVG images, editing of the title and description of an image, and adding rectangles and circles to SVG images. Any image that is uploaded to the site can be opened in the SVG image summary section to see all of its details.

This project taught me so much, I especially enjoyed the data manipulation that was required to make the backend run, specically doing it in a bare-bones language like C, which does not come with many libraries to make work such as this easier. Memory management and memory efficiency was hugely important as C does not have a garbage collector, so I had to be vigilant with my memory to avoid crashing the servers I wrote the back end on. Overall I feel this project made me a much more competent and confident programmer.



![SVG EDITOR pic 1](https://github.com/stuartvanp/SVGEditor/blob/master/svg_pics/svg1.png)

