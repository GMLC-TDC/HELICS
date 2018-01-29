class Helics < Formula
  desc "Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS)"
  homepage "https://github.com/GMLC-TDC/HELICS-src"
  url "https://github.com/GMLC-TDC/HELICS-src/archive/v1.0.0a.tar.gz"
  sha256 "df5a833a7cc12caf81ef894c67443bb7db34c9f069a6de16e3bc8b343865971f"
  head "https://github.com/GMLC-TDC/HELICS-src.git", :branch => "develop"

  bottle do
    cellar :any
  end

  depends_on "cmake" => :build
  depends_on "boost"
  depends_on "zeromq"
  depends_on "python" => :optional
  depends_on "python3" => :optional
  depends_on "swig" if build.with?("python")
  depends_on "swig" if build.with?("python3")

  def install

    ENV.O0
    mkdir "build" do
      args = std_cmake_args
      system "cmake", "..", *args
      system "make", "install"
    end
  end

  test do
    system "false"
  end
end


