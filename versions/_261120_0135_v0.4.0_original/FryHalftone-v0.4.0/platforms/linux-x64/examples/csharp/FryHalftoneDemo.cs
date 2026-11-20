// FryHalftoneDemo.cs —— ★393 C# / .NET 调用示例（P/Invoke）
//
// 【怎么编怎么跑】
//   dotnet new console -o FryDemo && 把本文件覆盖 Program.cs
//   把 fry_halftone.dll 放到生成目录（bin\Debug\net8.0\）旁边，然后：
//   dotnet run -- "D:\model.glb" "D:\out_slices"
//
// 【C# 对接这个库只有四个坑，都在下面标出来了】
//   ① 调用约定必须是 __stdcall（CallingConvention.StdCall）—— 写成默认的 Cdecl 在 x86 上会直接崩，
//      x64 上参数虽然能过，但是**别赌**：头文件写的就是 __stdcall。
//   ② 字符串走 UTF-8 的那一组接口（…Utf8）最省事：C# 的 string 是 UTF-16，
//      用 Marshal 传 byte[] 明确编码，不要让 runtime 猜。
//   ③ 进度回调必须用 GCHandle / 静态字段**保住委托**，否则 GC 把它回收掉 ⇒ 回调时进程崩。
//   ④ 返回码 0 才是成功；非 0 一定要读 errorBuffer（里面是中文原因）。
using System;
using System.Runtime.InteropServices;
using System.Text;

internal static class FryHalftone
{
    private const string Dll = "fry_halftone";          // 运行时找 fry_halftone.dll

    // ① 调用约定 __stdcall；② 自报家门返回的是库内静态 UTF-8 字符串，不要 free
    [DllImport(Dll, CallingConvention = CallingConvention.StdCall)]
    private static extern IntPtr FryHalftone_GetVersion();

    [DllImport(Dll, CallingConvention = CallingConvention.StdCall)]
    private static extern IntPtr FryHalftone_GetAlgorithmFingerprint();

    [DllImport(Dll, CallingConvention = CallingConvention.StdCall)]
    private static extern IntPtr FryHalftone_GetParamSchemaJson();

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void Utf8ProgressCallback(int percent, IntPtr stageUtf8, IntPtr userData);

    [DllImport(Dll, CallingConvention = CallingConvention.StdCall)]
    private static extern int FryHalftone_SliceModelJsonUtf8(
        byte[] requestJson, byte[] resultJson, int resultJsonBytes,
        Utf8ProgressCallback progressCallback, IntPtr userData,
        byte[] errorBuffer, int errorBufferBytes);

    [DllImport(Dll, CallingConvention = CallingConvention.StdCall)]
    public static extern int FryHalftone_Cancel();

    private static string Utf8(IntPtr p) => p == IntPtr.Zero ? "" : Marshal.PtrToStringUTF8(p) ?? "";
    private static byte[] Bytes(string s) => Encoding.UTF8.GetBytes(s + "\0");
    private static string Text(byte[] buf)
    {
        int n = Array.IndexOf(buf, (byte)0);
        return Encoding.UTF8.GetString(buf, 0, n < 0 ? buf.Length : n);
    }

    public static string Version() => Utf8(FryHalftone_GetVersion());
    public static string Fingerprint() => Utf8(FryHalftone_GetAlgorithmFingerprint());
    public static string ParamSchemaJson() => Utf8(FryHalftone_GetParamSchemaJson());

    // ③ 委托放静态字段：整个调用期间都不能被 GC 回收
    private static readonly Utf8ProgressCallback KeepAlive = (percent, stage, user) =>
        Console.WriteLine($"  {percent,3}%  {Utf8(stage)}");

    public static int Slice(string requestJson, out string resultJson, out string error)
    {
        var result = new byte[8192];                    // ⚠ 不够时返回 2，errorBuffer 会写明需要多少字节
        var err = new byte[2048];
        int rc = FryHalftone_SliceModelJsonUtf8(Bytes(requestJson), result, result.Length,
                                                KeepAlive, IntPtr.Zero, err, err.Length);
        resultJson = Text(result);
        error = Text(err);
        return rc;
    }
}

internal static class Program
{
    private static int Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;
        if (args.Length < 2)
        {
            Console.WriteLine("用法: FryHalftoneDemo <模型.glb> <输出目录>");
            return 2;
        }
        Console.WriteLine($"SDK 版本   : {FryHalftone.Version()}");
        Console.WriteLine($"算法指纹   : {FryHalftone.Fingerprint()}");

        // 参数白名单是库自己导出的（也就是包里的 params_schema.json）——界面可以据此自动生成
        string model = args[0].Replace("\\", "/");
        string outDir = args[1].Replace("\\", "/");
        string request = $@"{{
  ""model"": ""{model}"",
  ""out"":   ""{outDir}"",
  ""params"": {{
    ""target_longest_edge_mm"": 16.851,
    ""ink_c_um"": 400, ""ink_m_um"": 500, ""ink_y_um"": 300,
    ""band_uninked_fill"": ""transparent"",
    ""support_enable"": true,
    ""orient_mode"": ""auto_extent""
  }}
}}";
        int rc = FryHalftone.Slice(request, out string result, out string error);
        if (rc != 0)
        {
            Console.Error.WriteLine($"✗ 失败，返回码 {rc}：{error}");
            return rc;                                   // 7 = 这个模型用到了本库暂不支持的写法（原因在 error 里）
        }
        Console.WriteLine("✓ 成功：" + result);
        return 0;
    }
}
