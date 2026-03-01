/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== gesture_model_packaged.onnx ==================
static const unsigned char temp_binary_data_0[] =
"\x08\n"
"\x12\x07pytorch\x1a\t2.9.0+cpu:\xd5T\n"
"\xda\n"
"\n"
"\x05input\n"
"\x10network.0.weight\n"
"\x0enetwork.0.bias\x12\x06linear\x1a\x0bnode_linear\"\x04Gemm*\r\n"
"\x06transA\x18\0\xa0\x01\x02*\r\n"
"\x06transB\x18\x01\xa0\x01\x02*\x0f\n"
"\x05""alpha\x15\0\0\x80?\xa0\x01\x01*\x0e\n"
"\x04""beta\x15\0\0\x80?\xa0\x01\x01J\x97\x01\n"
"\tnamespace\x12\x89\x01: __main__.GestureNN/network: torch.nn.modules.container.Sequential/network.0: torch.nn.modules.linear.Linear/linear: aten.linear.defaultJ\x9a\x01\n"
"\x1epkg.torch.onnx.class_hierarchy\x12x['__main__.GestureNN', 'torch.nn.modules.container.Sequential', 'torch.nn.modules.linear.Linear', 'aten.linear.default']J\xaa\x01\n"
"\x16pkg.torch.onnx.fx_node\x12\x8f\x01%linear : [num_users=1] = call_function[target=torch.ops.aten.linear.default](args = (%x, %p_network_0_weight, %p_network_0_bias), kwargs = {})JD\n"
"\x1apkg.torch.onnx.name_scopes\x12&['', 'network', 'network.0', 'linear']J\xaa\x05\n"
"\x1apkg.torch.onnx.stack_trace\x12\x8b\x05""File \"C:\\Users\\Student\\Desktop\\Jacks Desktop\\GestureInstrument\\Gesture Instrument\\Gesture Instrument\\Analysis\\train_model.py\", line 96, in forward\n"
"    return self.network(x)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\container.py\", line 250, in forward\n"
"    input = module(input)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\linear.py\", line 134, in forward\n"
"    return F.linear(input, self.weight, self.bias)\n"
"\xcb\t\n"
"\x06linear\x12\x04relu\x1a\tnode_relu\"\x04ReluJ\x95\x01\n"
"\tnamespace\x12\x87\x01: __main__.GestureNN/network: torch.nn.modules.container.Sequential/network.1: torch.nn.modules.activation.ReLU/relu: aten.relu.defaultJ\x9a\x01\n"
"\x1epkg.torch.onnx.class_hierarchy\x12x['__main__.GestureNN', 'torch.nn.modules.container.Sequential', 'torch.nn.modules.activation.ReLU', 'aten.relu.default']J\x83\x01\n"
"\x16pkg.torch.onnx.fx_node\x12i%relu : [num_users=1] = call_function[target=torch.ops.aten.relu.default](args = (%linear,), kwargs = {})JB\n"
"\x1apkg.torch.onnx.name_scopes\x12$['', 'network', 'network.1', 'relu']J\xaa\x05\n"
"\x1apkg.torch.onnx.stack_trace\x12\x8b\x05""File \"C:\\Users\\Student\\Desktop\\Jacks Desktop\\GestureInstrument\\Gesture Instrument\\Gesture Instrument\\Analysis\\train_model.py\", line 96, in forward\n"
"    return self.network(x)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\container.py\", line 250, in forward\n"
"    input = module(input)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\activation.py\", line 144, in forward\n"
"    return F.relu(input, inplace=self.inplace)\n"
"\xe6\n"
"\n"
"\x04relu\n"
"\x10network.2.weight\n"
"\x0enetwork.2.bias\x12\x08linear_1\x1a\rnode_linear_1\"\x04Gemm*\r\n"
"\x06transA\x18\0\xa0\x01\x02*\r\n"
"\x06transB\x18\x01\xa0\x01\x02*\x0f\n"
"\x05""alpha\x15\0\0\x80?\xa0\x01\x01*\x0e\n"
"\x04""beta\x15\0\0\x80?\xa0\x01\x01J\x99\x01\n"
"\tnamespace\x12\x8b\x01: __main__.GestureNN/network: torch.nn.modules.container.Sequential/network.2: torch.nn.modules.linear.Linear/linear_1: aten.linear.defaultJ\x9a\x01\n"
"\x1epkg.torch.onnx.class_hierarchy\x12x['__main__.GestureNN', 'torch.nn.modules.container.Sequential', 'torch.nn.modules.linear.Linear', 'aten.linear.default']J\xaf\x01\n"
"\x16pkg.torch.onnx.fx_node\x12\x94\x01%linear_1 : [num_users=1] = call_function[target=torch.ops.aten.linear.default](args = (%relu, %p_network_2_weight, %p_network_2_bias), kwargs = {})JF\n"
"\x1apkg.torch.onnx.name_scopes\x12(['', 'network', 'network.2', 'linear_1']J\xaa\x05\n"
"\x1apkg.torch.onnx.stack_trace\x12\x8b\x05""File \"C:\\Users\\Student\\Desktop\\Jacks Desktop\\GestureInstrument\\Gesture Instrument\\Gesture Instrument\\Analysis\\train_model.py\", line 96, in forward\n"
"    return self.network(x)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\container.py\", line 250, in forward\n"
"    input = module(input)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\linear.py\", line 134, in forward\n"
"    return F.linear(input, self.weight, self.bias)\n"
"\xd9\t\n"
"\x08linear_1\x12\x06relu_1\x1a\x0bnode_relu_1\"\x04ReluJ\x97\x01\n"
"\tnamespace\x12\x89\x01: __main__.GestureNN/network: torch.nn.modules.container.Sequential/network.3: torch.nn.modules.activation.ReLU/relu_1: aten.relu.defaultJ\x9a\x01\n"
"\x1epkg.torch.onnx.class_hierarchy\x12x['__main__.GestureNN', 'torch.nn.modules.container.Sequential', 'torch.nn.modules.activation.ReLU', 'aten.relu.default']J\x87\x01\n"
"\x16pkg.torch.onnx.fx_node\x12m%relu_1 : [num_users=1] = call_function[target=torch.ops.aten.relu.default](args = (%linear_1,), kwargs = {})JD\n"
"\x1apkg.torch.onnx.name_scopes\x12&['', 'network', 'network.3', 'relu_1']J\xaa\x05\n"
"\x1apkg.torch.onnx.stack_trace\x12\x8b\x05""File \"C:\\Users\\Student\\Desktop\\Jacks Desktop\\GestureInstrument\\Gesture Instrument\\Gesture Instrument\\Analysis\\train_model.py\", line 96, in forward\n"
"    return self.network(x)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\container.py\", line 250, in forward\n"
"    input = module(input)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\activation.py\", line 144, in forward\n"
"    return F.relu(input, inplace=self.inplace)\n"
"\xe8\n"
"\n"
"\x06relu_1\n"
"\x10network.4.weight\n"
"\x0enetwork.4.bias\x12\x06output\x1a\rnode_linear_2\"\x04Gemm*\r\n"
"\x06transA\x18\0\xa0\x01\x02*\r\n"
"\x06transB\x18\x01\xa0\x01\x02*\x0f\n"
"\x05""alpha\x15\0\0\x80?\xa0\x01\x01*\x0e\n"
"\x04""beta\x15\0\0\x80?\xa0\x01\x01J\x99\x01\n"
"\tnamespace\x12\x8b\x01: __main__.GestureNN/network: torch.nn.modules.container.Sequential/network.4: torch.nn.modules.linear.Linear/linear_2: aten.linear.defaultJ\x9a\x01\n"
"\x1epkg.torch.onnx.class_hierarchy\x12x['__main__.GestureNN', 'torch.nn.modules.container.Sequential', 'torch.nn.modules.linear.Linear', 'aten.linear.default']J\xb1\x01\n"
"\x16pkg.torch.onnx.fx_node\x12\x96\x01%linear_2 : [num_users=1] = call_function[target=torch.ops.aten.linear.default](args = (%relu_1, %p_network_4_weight, %p_network_4_bias), kwargs = {})JF\n"
"\x1apkg.torch.onnx.name_scopes\x12(['', 'network', 'network.4', 'linear_2']J\xaa\x05\n"
"\x1apkg.torch.onnx.stack_trace\x12\x8b\x05""File \"C:\\Users\\Student\\Desktop\\Jacks Desktop\\GestureInstrument\\Gesture Instrument\\Gesture Instrument\\Analysis\\train_model.py\", line 96, in forward\n"
"    return self.network(x)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\container.py\", line 250, in forward\n"
"    input = module(input)\n"
"  File \"C:\\Users\\Student\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python312\\site-packages\\torch\\nn\\modules\\linear.py\", line 134, in forward\n"
"    return F.linear(input, self.weight, self.bias)\x12\n"
"main_graph*\x9d\x03\x08\x18\x08\x04\x10\x01""B\x10network.0.weightJ\x80\x03K\xd2L>\xc1\xc4?>r\xc0""D>L\x03\xe3\xbe""E\x9d\xcf>\xf2h2\xbf\x86\xfd\xd0>_\\9\xbf\x10\xda""e>\xf9\xcb\xaa\xbe_\x92p\xbe:N\x9a>,|\xa9\xbe\xe4\xf0\x05\xbe""5}\xa9>\x04~\xaa=\xfb"
";\x0f<\xb1\xd4O\xbe\x83\x03\xe6= \xc7N\xbe\n"
"K\x97>\xfd\x9f\xa4\xbe\xce\x10U\xbe\xa1\xa3\">\xda.\x19\xbf""7\xe9\xd8\xbe\x1d\0\x05>}\xa9t=\xb5""f.>\xdeI@\xbe\x12\xdd)\xbe\x96t\xd4>\xe8oF>\x8a\x97\xa3\xbe\x86\x0b\xd0>\x9b\0\xf7>\x84.\xf9>\x1e$\x05>Z\x84\x9d\xbe\xd1\x8c\x1d=z\xe2""d>\xc9\xef\x05>\x91"
"\x88\xae\xbe""d\xb2\xdd<<<\xbd>\x8b\xd1%\xbd\xd2\x82->\xe0""5\x11>\xb1\xe1\x1b\xbe""e\x86\x10\xbd\x17\xc4m>b{\xc6>\x15u\x08\xbf\xe2\xa9\x8f\xbe\xd9\xd6\x06\xbe\xaa\xf4\xc4>S\x1e""f\xbd\xa3\xa7\xb7>\x85\xfb!\xbf\xb7\x08\xd8\xbe\xab\xef\xe4\xbe""b\xb0\xa3"
"=\xbdR\x92>Z\xd2\xd8>6@\x81><\x07\xb7\xbd\xd1""1\x12\xbf\xdb\xf4\xc3\xbe|+\xfa\xbdO\xa7\x05>\x98""3\x8d>y\xb8\x9e\xbe\xa6\xc7\xcf\xbd""9\xbc\x9e>\xc8\xb6\xef\xbe\x1c<\xd0\xbe\x9b\xb7*\xbf""C\x97\xcf\xbe\xad\x10""4=\x88@s>\xc5\xceW>A\xac<>\xe0\xa4y=\xba"
"Wy\xbd\xfc\xb6]\xbe\xecR\x1b\xbe\xa6\x95\xf5\xbe(\x87\xfc\xbe\xf6\xe0\x8f\xbeN\xbe\x90\xbe\xb0\xb9\x18\xbeHX\x85\xbe\xb6\x83\xb1>\\Z\xa8>\xb9S\xb6>\xd3\x10\x01\xbep\0*v\x08\x18\x10\x01""B\x0enetwork.0.biasJ`Ih\x14\xbf\xa5&\xb6:r\xf8\xf9>b~*?\n"
"\xed\x99?.\xd2\x84\xbe\x1d\xe0\xc6>\ru\x06\xbf:/\x9b=u\xf5\x7f\xbf*\x11\x9c?\x1ds\x9a?\xb7\xa4""c\xbeg\xe6g=\xc0\"\xd8=K\x9e\xb3;q!\xf4>\x1f\x82r=\xd6q\x0f\xbf""f\xb3g>\x11\xec\x0c\xbf\xba\xc8\xd5>\x10X\xcb>!\xbam\xbf*\x9d\t\x08\x0c\x08\x18\x10\x01""B"
"\x10network.2.weightJ\x80\t\xab\xb1U\xbdk\xeb&=?\xbb\x03\xbe\xeb\x87\xd1:*\xb4\x15=3\xf2\x02\xbeR>\t\xbe^\xe0\xbf=\xe2\x85\xd1\xbdZ&\xf3=\xc7+\xe5\xbc\x97v3\xbc""a\x7f!>\xb2\xdc\xd8=,nF\xbdm\xb2\x0e=\xbd\0\xaa\xbc@\x16\xbe=*u*\xbe>0N\xbe\xb3P.>X3\xdd"
"<\xbc\x01\x9e=\xdb;\xdb\xbd\x15~\x9d=\xb4\xd0\x9a=]\n"
"\xbc\xbc\xff\xf9\xdc<T\x95\xd6\xbd\xad\0\x93\xbe?\xb9w\xbe\xde\x0c\xce<\xcd\xdd\x11>7\x86\xe6=\xfa\x11_>\xdeN\r<\xe3\xa3~\xbd\xc2\xc2\x05\xbe\xb8\xd8\x1f>\xe7P\x8a\xbc;ex\xbe\xe4\xa2""3=t\xff\0\xbe\x8eP\x8b\xbe\x91\xd9\r>\x99t\xa3\xbd\x1aRE\xbeo\xfb\xca"
"\xbdY9)>\xc5\x90V\xbe\x1e\xa6!=~h\xa9=\x9b\x19S\xbeO\x94""7\xbb$\xc5\xf4=\xb5\x07""5\xbe""7\xa1\x02\xbf\xfc\xe7(\xbc\x1a\xfb{\xbd\xe2""6\x92\xbc\\\xb1\x91\xbe\xd4Q\x15\xbe\x8dgI=\xe3\x0fH\xbe\xa1""2\x16\xbc""eW\xbc\xbd \xf2\n"
"\xbbR\xf2\x81\xbd\xedl$\xbe&j\xf5\xbd\xc7\0\xdb\xbd\x84\x93L>\x82\x0b\x17\xbeplX=@\xd1""A=\xd6\xa1""1\xbe`U\xbc\xbc\xa1:+\xbe \x02\xb5\xbcl|3\xbe\x01\xfd,\xbe""0aA\xbc\x04;B\xbe\x88\x14\xc5=<\xa4\x17=\x1e~\x1f\xbe\x92\xad?>\xa0\xc7""c=\x11/\xec\xbd\xf0"
"\xf7\x8a<\x8emI\xbe\xc0\xbe""e;\xee\xa7\x15>\xe0""D9\xbc\x94x\xd9\xbdu\x16""C\xbe\x9fUg\xbe\xa4lB=\xd4iB>\xd2\x99\x11\xbe\xe7]\xb8\xbcj1/=\xb5,\xac\xbeH\xc4\x82\xbd\xe4$\xa5\xbd\xbdu\x8a=\x8e\x91\xf8=\x93\xcb\x19>\xf7\x8b\xf4=?k\xf4\xbc""EY6\xbe\x89\xaf"
"\xcc\xbc\xa8""ek\xbeh\xee+>\xd1\xd2\x10>Mi\xbc\xbd\x15\xb4\xdc=\xd4\xb0\xde=\x88\x94\xef=\xf7\0\x08>-E\xac=\xf9""c\x12>\xd6\xe4\x9a""9\xa7""3\x07\xbe\xe6J,=\x7f""b\xaf\xbd""7v\xfc;\xcf\xe1\xc4\xbd\xa8\xcf""4\xbd\xd5\x16\x9e=\xb7\x87\x1b=\xbay5\xbc\x90"
"\xaf\x12>\x82""fP==fK>\x14\xa3\x15>\n"
"?Q\xbd\xc6\x8fK\xbe\xfa,^>O}Y>\xc9\0*>\x0e\0M>\xe0\x06""A>|_\n"
"\xbd\xfe\x10\x7f\xbe\x8c\xb0\xe0\xbe\xae\x1c\xec<\x06\x88\x8f=\xb8""BE?\xaa\x19N\xbd\xe6<\x06?\xbb\x7fy\xbe\x86W.\xbe\xc3jn\xbe\xf1\xf1\xae>\xbe\\!>\0*I=\xb1\"\xd7<s@$>2/j\xbd.A\xee=or\xb3\xbe""e\x1e\xd0=\x99\xba}>7\x13\x06>\xd0\xf5""0=\xca[D>\xb1\\\x17"
"\xbe\xab\xb1\x1a>\x1f:\xa2\xbd\xb0\x07\x1c=\xcd""25>o7!\xbe\x96\x13W=E\xd0w\xbe\xc0\xc9\xad\xbd\x11W\x06>6\xedM\xbe/Y\x16>j\xa8\r>\x14\xcf""2<\x1ai\x8f\xbe|\x12\xb6\xbd\x96\x03$\xbe\xa3Y\x02\xbe\xfe\xf2\xda\xbcH\xc8\n"
">w\xb9S\xbe\xf1M*=\xc8\xbc\xed=\xd8\x15""B=\x85)\xd1=\xfcw0>p\xb6\xfa\xbc\x99{,\xbe""5j\x07\xbd\r\xbdI<Aw\x1b>\xaf\x05\xa1\xbc\xbbsB=1|\xd1\xbd\xbf(\xea\xbc.6\xb9;k\x8aR\xbe\xe8\xd6\x18\xbeK\x08(>\xd2\xcc<=\xfe\xe6/>\x9f\xa2\x19\xbdGYF>k\x0f\x8a=L\xeb"
"\xce\xbc\x04\xc1""5<x8\xef\xbckP\xde\xbd\n"
"\x9c\xe9\xbd""AS\x0e\xbe\x1cGc>\x83l\x88\xbe\x02\xb4\xcb\xbc\xaa%\xa9\xbd\xc6q?>\xd5\xce\x95>\xc4Hs\xbc\xe0xh\xbdV\x18#\xbd\x8a|'\xbe%\x06""d;\x88\xd9\x12=\xcf\xe0\xd5=V\xbd\x0f\xbc\xb7""D&=\xc3]>>\xd7\xa1\x10\xbe\xc9\x19\x91\xbd""3c\x95=\xcf\xd7""A>"
" \x05&\xbeI\xaeO\xbe:\x11""8>$\xe7(\xbe\xde\xfc\x17>\xa9\xa2\x9c>\xd4\xa7\xae=z&\xf6\xbd\x91\xfan>\xb8\xe9\x8f\xbe""4\xe6\x85=\x1a\x9c\t>uu\\\xbe\x04Q\xc0\xbe""FJ\xa5\xbd\xff\x9bi=\x87\xa2\xed\xbc\xcf""d\xcf=1_+\xbd\xb9{\x85\xbb,n\x92;\xe2\xfe\x87\xbd"
"\xc4\x85\x1f\xbe\xbd\xd3\x12\xbe\x1a%\x1f>\xf9\x14+\xbe\xb5)\n"
"\xbd\x9f""2K\xbeY\xd8\xfc\xbd""de\x10\xbdR\xa4(\xbe\xf9\x98""A>\xb2\x92\xcb\xbe\xaeU{>\xaa\xb2\xc7\xbb}c\xb7=\x12(\x8c\xbd\x94\x05\x1e>x\x98%\xbc\x86\xf6\x1a=\x03\xd9\xa6\xbd\xf9""b\x9c\xbe\xf6\xef+\xbd\n"
"Q\xb8\xbe\0\xcb\xc2\xbd\xae\xee""F\xbe\xa2+\x90=\xfbU\xdc=\xb8\xc1\x1e\xbe\xfd\x12L\xbe\xb3Uo\xbdp\0*F\x08\x0c\x10\x01""B\x0enetwork.2.biasJ0\xec|\xe6=0\x86!?`)\xdb\xbd""6U\x16>\r}\xf9>\xc1\x1c\xea\xbe\x86\xe8""b?HX\x91>9\xe8\xd2;\xc8\x89\xeb\xbe\xa4"
"\xc7\xf1>`\x8c\x11>*z\x08\x02\x08\x0c\x10\x01""B\x10network.4.weightJ`\xd2n\x93\xbdw\x01h\xbex$\xad\xbd|\x98\xf0=\x11\x06i\xbe""BGx>\xd4\x1dP?$}v\xbc\xd6\xf5\x06\xbe\xd4r\xc4<\x1e\xc1""6\xbe!\x12\xd3\xbd\x02\xc5""e\xbd\xbb""4`\xbd.y\x8f\xbe`\xf0\x1d<"
"\xcf""C&\xbd.\x86J\xbe\xde\xd6\x8c\xbf\xf7/\x15>\tAV\xbe\x10\x07\x96\xbe\xb5\x98\xd6=\x18\xf9\xd6\xbd*\x1e\x08\x02\x10\x01""B\x0enetwork.4.biasJ\x08 \\!\xbe\xcf\xf5\x19?Z\xc0\x01\n"
"\x05input\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x04\"=\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\n"
"USER_INPUT\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"&\n"
"!pkg.torch.onnx.original_node_name\x12\x01xb\x8b\x01\n"
"\x06output\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x02\"?\n"
"0pkg.torch.export.graph_signature.OutputSpec.kind\x12\x0bUSER_OUTPUT\"-\n"
"!pkg.torch.onnx.original_node_name\x12\x08linear_2j\xd8\x01\n"
"\x10network.0.weight\x12\x0e\n"
"\x0c\x08\x01\x12\x08\n"
"\x02\x08\x18\n"
"\x02\x08\x04\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"7\n"
"!pkg.torch.onnx.original_node_name\x12\x12p_network_0_weightj\xd0\x01\n"
"\x0enetwork.0.bias\x12\n"
"\n"
"\x08\x08\x01\x12\x04\n"
"\x02\x08\x18\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"5\n"
"!pkg.torch.onnx.original_node_name\x12\x10p_network_0_biasj\xd8\x01\n"
"\x10network.2.weight\x12\x0e\n"
"\x0c\x08\x01\x12\x08\n"
"\x02\x08\x0c\n"
"\x02\x08\x18\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"7\n"
"!pkg.torch.onnx.original_node_name\x12\x12p_network_2_weightj\xd0\x01\n"
"\x0enetwork.2.bias\x12\n"
"\n"
"\x08\x08\x01\x12\x04\n"
"\x02\x08\x0c\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"5\n"
"!pkg.torch.onnx.original_node_name\x12\x10p_network_2_biasj\xd8\x01\n"
"\x10network.4.weight\x12\x0e\n"
"\x0c\x08\x01\x12\x08\n"
"\x02\x08\x02\n"
"\x02\x08\x0c\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"7\n"
"!pkg.torch.onnx.original_node_name\x12\x12p_network_4_weightj\xd0\x01\n"
"\x0enetwork.4.bias\x12\n"
"\n"
"\x08\x08\x01\x12\x04\n"
"\x02\x08\x02\"<\n"
"/pkg.torch.export.graph_signature.InputSpec.kind\x12\tPARAMETER\"=\n"
"5pkg.torch.export.graph_signature.InputSpec.persistent\x12\x04None\"5\n"
"!pkg.torch.onnx.original_node_name\x12\x10p_network_4_biasj\x1b\n"
"\x06linear\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x18j\x19\n"
"\x04relu\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x18j\x1d\n"
"\x08linear_1\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x0cj\x1b\n"
"\x06relu_1\x12\x11\n"
"\x0f\x08\x01\x12\x0b\n"
"\x05\x12\x03s77\n"
"\x02\x08\x0c\x82\x01\xb2\x03\n"
"0pkg.torch.export.ExportedProgram.graph_signature\x12\xfd\x02\n"
"# inputs\n"
"p_network_0_weight: PARAMETER target='network.0.weight'\n"
"p_network_0_bias: PARAMETER target='network.0.bias'\n"
"p_network_2_weight: PARAMETER target='network.2.weight'\n"
"p_network_2_bias: PARAMETER target='network.2.bias'\n"
"p_network_4_weight: PARAMETER target='network.4.weight'\n"
"p_network_4_bias: PARAMETER target='network.4.bias'\n"
"x: USER_INPUT\n"
"\n"
"# outputs\n"
"linear_2: USER_OUTPUT\n"
"\x82\x01J\n"
"2pkg.torch.export.ExportedProgram.range_constraints\x12\x14{s77: VR[0, int_oo]}B\x04\n"
"\0\x10\x14";

const char* gesture_model_packaged_onnx = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x9d2adc1e:  numBytes = 10868; return gesture_model_packaged_onnx;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "gesture_model_packaged_onnx"
};

const char* originalFilenames[] =
{
    "gesture_model_packaged.onnx"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}
