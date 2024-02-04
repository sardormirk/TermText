<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->



<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Issues][issues-shield]][issues-url]




<!-- PROJECT LOGO -->
<br />

<h3 align="center">TermText - A pure C minimalistic Terminal text-editor with ~1.2 LOC </h3>

  <p align="center">
   A barebones C terminal text editor with almost 1200 lines of code. Written in pure C with no external libraries, it features a Vim-like feature set of keyboard movements and commands, syntax highlighting, file creation/loading, text editing, modal editing (normal mode, insert mode), and a work-in-progress config system.
    <br />
    <a href="https://github.com/voidblob/TermText"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/voidblob/TermText">View Demo</a>
    ·
    <a href="https://github.com/voidblob/TermText/issues">Report Bug</a>
    ·
    <a href="https://github.com/voidblob/TermText/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project
![Showcase](https://raw.githubusercontent.com/voidblob/TermText/master/showcase.png)

<h5>Key Features</h5>
<ol>
  <li>Vim-like Features: Efficient keyboard movements and commands for streamlined editing.</li>
  <li>Enhance code readability with syntax highlighting.</li>
  <li>Text Editing: Core functionality for manipulating and modifying text.</li>
  <li>
WIP Config System: Work-in-progress configuration system for customization.</li>
</ol>




<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

* [![C++][C++]][C++-url]
* [![Make][Cmake]][Cmake-url]

  
<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

The following instructions are a general installation guide for MacOS/Linux. To install on windows, download visual studio and create a new project using this repo's code as existing files. 

### Prerequisites


* Cmake
  MacOS/Linux
  ```sh
  brew install cmake
  sudo apt-get install cmake
  ```

### Installation


1. Clone the repo
   ```sh
   git clone https://github.com/voidblob/TermText.git
   ```
2. Go into the Build directory and run the following commands
   ```sh
   cmake ..
   make
   ./TermText #no file name creates a new file
   ./TermText #file_name - opens existing file
   ```
<h5>On Windows</h5>

1. Download a linux distribution using WSL

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

To use the text editor, simply call ./TermText from the build directory along with a file name or leave it blank to create a new file

1. Adding syntax highlighting for other languages:
```cpp
char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL}; //Add file extensions (.js for javascript, etc)

char *C_HL_keywords[] = { //Add keywords for the language you want to highlight, adding a | makes it a secondary keyword (different colour)
    "#include",
    "switch",
    "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL};

struct editorSyntax HLDB[] = {
    {"c", C_HL_extensions, C_HL_keywords, "//", "/*", "*/", HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
}; //Create a editorSyntax struct, passing in the filetype, extensions array, keywords array, comment structure (C usings // and /* for comments), and highlight flags
```

2. Commands:

* END: goes to end of line
* HOME: goes to beginning of line
* PG_DOWN: goes to end of file
* PG_UP: goes to beginning of file
* Arrow keys: movement
* HJKL: movement
* DD: deletes a line
* CTRL_S: saves file
* CTRL_Q: quits file
* ESC: enters normal mode
* i: enters insert mode




<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [ ] Finish config system
- [ ] Add more colour options for different keywords
- [ ] Add a file loading system for the file extension arrays and keyword database


See the [open issues](https://github.com/voidblob/TermText/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>








<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/voidblob/TermText.svg?style=for-the-badge
[contributors-url]: https://github.com/voidblob/TermText/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/voidblob/TermText.svg?style=for-the-badge
[forks-url]: https://github.com/voidblob/TermText/network/members
[stars-shield]: https://img.shields.io/github/stars/voidblob/TermText.svg?style=for-the-badge
[stars-url]: https://github.com/voidblob/TermText/stargazers
[issues-shield]: https://img.shields.io/github/issues/voidblob/TermText.svg?style=for-the-badge
[issues-url]: https://github.com/voidblob/TermText/issues
[license-shield]: https://img.shields.io/github/license/voidblob/TermText.svg?style=for-the-badge
[license-url]: https://github.com/voidblob/TermText/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username
[product-screenshot]: https://github.com/voidblob/TermText/blob/master/demo.png
[C++]: https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white
[C++-url]: https://cplusplus.com/
[OpenGL]: https://img.shields.io/badge/OpenGL-%23FFFFFF.svg?style=for-the-badge&logo=opengl
[OpenGL-url]: https://www.opengl.org/
[Cmake]: https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white
[Cmake-url]: https://cmake.org/
[Angular.io]: https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white
[Angular-url]: https://angular.io/
[Svelte.dev]: https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00
[Svelte-url]: https://svelte.dev/
[Laravel.com]: https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white
[Laravel-url]: https://laravel.com
[Bootstrap.com]: https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white
[Bootstrap-url]: https://getbootstrap.com
[JQuery.com]: https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white
[JQuery-url]: https://jquery.com 
