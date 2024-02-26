#include <yuser.h>

#define MAX_INPUT 100

int
main(int argc, char** argv)
{
  char* input = malloc(MAX_INPUT * sizeof(char));
  TtyRead(0, input, MAX_INPUT);
  TtyPrintf(0, "input: %s\n", input);

  TtyPrintf(0, "another line\n");
  TtyRead(0, input, MAX_INPUT);
  TtyRead(0, input, MAX_INPUT);
  TtyPrintf(0, "input: %s\n", input);

  TtyPrintf(2, "Let's put something on terminal two\n");
  TtyRead(2, input, MAX_INPUT);
  TtyPrintf(1, "Got something on terminal 2\n");

  TtyPrintf(0, "Here's a long ass string: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Nisl nunc mi ipsum faucibus vitae aliquet. Sem fringilla ut morbi tincidunt augue interdum velit euismod. Augue ut lectus arcu bibendum at varius. Sed libero enim sed faucibus turpis in eu. Id cursus metus aliquam eleifend mi in nulla posuere sollicitudin. A iaculis at erat pellentesque adipiscing commodo elit at imperdiet. Aenean et tortor at risus viverra adipiscing at. Velit euismod in pellentesque massa placerat duis ultricies lacus sed. Iaculis urna id volutpat lacus laoreet non. Sem fringilla ut morbi tincidunt augue interdum. Diam phasellus vestibulum lorem sed risus. Elit sed vulputate mi sit amet mauris commodo. Tempus egestas sed sed risus pretium quam vulputate dignissim. In aliquam sem fringilla ut morbi. Purus gravida quis blandit turpis cursus in hac. Est placerat in egestas erat imperdiet sed euismod. Nisl suscipit adipiscing bibendum est ultricies. Nunc consequat interdum varius sit amet. Quisque non tellus orci ac auctor augue mauris augue neque. Aliquam ultrices sagittis orci a scelerisque purus semper. Sollicitudin nibh sit amet commodo nulla facilisi. Urna cursus eget nunc scelerisque viverra mauris in aliquam. Maecenas accumsan lacus vel facilisis. Pellentesque elit ullamcorper dignissim cras tincidunt. Facilisi morbi tempus iaculis urna id volutpat lacus. Mattis nunc sed blandit libero volutpat. Turpis egestas sed tempus urna et pharetra pharetra massa massa. Eu facilisis sed odio morbi quis commodo odio aenean sed. Justo eget magna fermentum iaculis eu non diam phasellus. Justo eget magna fermentum iaculis eu non diam phasellus vestibulum. Nibh venenatis cras sed felis eget velit aliquet sagittis id. Est lorem ipsum dolor sit amet consectetur. Sit amet volutpat consequat mauris nunc. In dictum non consectetur a erat. Quis viverra nibh cras pulvinar mattis. Quisque id diam vel quam elementum pulvinar etiam. Placerat in egestas erat imperdiet sed euismod. Neque gravida in fermentum et sollicitudin ac orci phasellus. Urna neque viverra justo nec ultrices dui sapien. Accumsan tortor posuere ac ut consequat. A arcu cursus vitae congue mauris rhoncus aenean vel. Nec nam aliquam sem et tortor consequat id porta. Hendrerit dolor magna eget est lorem ipsum dolor. Feugiat sed lectus vestibulum mattis. Mus mauris vitae ultricies leo integer malesuada nunc vel. A arcu cursus vitae congue mauris rhoncus aenean vel. Enim diam vulputate ut pharetra sit. Viverra vitae congue eu consequat ac. Suscipit tellus mauris a diam maecenas sed enim ut. Dictum varius duis at consectetur lorem donec. Non sodales neque sodales ut etiam. Ornare suspendisse sed nisi lacus sed viverra tellus in hac. Integer feugiat scelerisque varius morbi enim nunc. Convallis a cras semper auctor neque vitae tempus quam pellentesque. Ante in nibh mauris cursus mattis molestie a iaculis. Tristique magna sit amet purus gravida quis. Scelerisque in dictum non consectetur. Malesuada proin libero nunc consequat interdum varius. Sit amet consectetur adipiscing elit pellentesque habitant. Pulvinar mattis nunc sed blandit libero volutpat sed cras. Aenean sed adipiscing diam donec adipiscing tristique risus nec. Laoreet suspendisse interdum consectetur libero. Viverra suspendisse potenti nullam ac tortor vitae purus faucibus ornare. Venenatis tellus in metus vulputate. Maecenas accumsan lacus vel facilisis volutpat est velit.\n");

  Delay(20);
}
