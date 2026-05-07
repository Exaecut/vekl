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

/// Cap on the mip level count carried in `TextureDesc`. Must match the
/// host-side `MAX_MIP` constant in prgpu's `types::config`. Five levels
/// cover 1/16 per axis which is deep enough for any sweep / blur pyramid;
/// it keeps the descriptor small (four `uint[5]` arrays = 80 extra bytes).
public static const uint MAX_MIP = 5u;

public struct TextureDesc
{
    public uint width;
    public uint height;
    public uint pitchBytes;
    public uint bytesPerPixel;
    public PixelStorage storage;
    public PixelLayout layout;
    public AddressMode addressMode;

    // Mip-chain metadata. `mipLevelCount >= 1`; entries beyond that are
    // undefined. Level 0 mirrors `width / height / pitchBytes` above.
    // The host fills these whenever it allocates a mip-chain buffer.
    // Effects that never request mips see `mipLevelCount == 1` and can
    // ignore these fields entirely.
    public uint mipLevelCount;
    public uint mipOffsetBytes[MAX_MIP];
    public uint mipWidth[MAX_MIP];
    public uint mipHeight[MAX_MIP];
    public uint mipPitchBytes[MAX_MIP];
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
