implementing vekl;

public enum PixelStorage : uint
{
    Unorm8x4  = 0,
    Unorm16x4 = 1,
    Float32x4 = 2,
}

public enum PixelLayout : uint
{
    Rgba    = 0,
    Bgra    = 1,
    Vuya    = 2,
    Vuya709 = 3,
}

public enum AddressMode : uint
{
    Clamp  = 0,
    Repeat = 1,
    Mirror = 2,
}

public struct TextureDesc
{
    public uint width;
    public uint height;
    public uint pitchBytes;
    public uint bytesPerPixel;
    public PixelStorage storage;
    public PixelLayout layout;
    public AddressMode addressMode;
}

public struct FrameParams
{
    public TextureDesc outDesc;
    public TextureDesc inDesc;
    public TextureDesc dstDesc;
    public uint width;
    public uint height;
    public float time;
    public float progress;
}
