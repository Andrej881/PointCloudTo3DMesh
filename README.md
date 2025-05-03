# PointCloudTo3DMesh

## Setup
### Prerequisites
  <p>Visual Studio 2022 (or later) with the C++ Desktop Development workload<br>  
  vcpkg installed and available in your system PATH</p>
  
### Install Dependencies

  <p>This project uses vcpkg in manifest mode. You do not need to install packages manually.<br>  
  If you haven’t used vcpkg before, clone and bootstrap it:</p>
  
  <p>git clone https://github.com/microsoft/vcpkg.git<br>
  cd vcpkg<br>  
  ./bootstrap-vcpkg.bat</p>

  <p>Then, return to your project root and integrate vcpkg:</p>

  <p>vcpkg integrate install<br>
  This enables Visual Studio to automatically use vcpkg and read the vcpkg.json file to install required dependencies.</p>

  <p>Open the Project<br>
  Open the .sln file in Visual Studio.<br>
  Build the project (Ctrl+Shift+B).</p>

  <p>Visual Studio will automatically detect the vcpkg.json and use vcpkg to resolve dependencies.</p>
