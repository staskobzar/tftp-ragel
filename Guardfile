# Guardfile
#
guard :shell do
  watch(/configure\.ac/) do
    system("./bootstrap")
  end
  watch(/Makefile\.am/) do
    system("make clean")
    system("./configure --with-coverage")
  end
  watch(/Doxyfile\.in/) do
    system("make")
  end
  watch(/(src|test)\/(.+)\.(c|h)/) do
    system("make clean")
    system("make")
    system("make check || cat test/*_test.log")
  end
  watch(/test\/frontend\/.+\.exp/) do
    system("make check || cat test/*_test.log")
  end
  watch(%r{(src\/lib\/tftp\.rl)}) do |m|
    system("ragel #{m[1]}")
  end
end
