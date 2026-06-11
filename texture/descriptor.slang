implementing vekl;

public enum PixelStorage : uint
{
    Unorm8x4  = 0,
    Unorm16x4 = 1,
    Float32x4 = 2,
    Float16x4 = 3,
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

/// Cap on `TextureDesc` mip levels. Must equal prgpu's host-side `MAX_MIP`.
/// Seven levels covers 1/64 per axis.
public static const uint MAX_MIP = 7u;

public struct TextureDesc
{
    public uint width;
    public uint height;
    public uint pitchBytes;
    public uint bytesPerPixel;
    public PixelStorage storage;
    public PixelLayout layout;
    public AddressMode addressMode;
    // 0 = top-down (row 0 = top). 1 = bottom-up host buffer (Premiere CPU): the
    // sampler maps row 0 to the last memory row so UV stays top-left like the GPU.
    public uint flipY;

    // Mip-chain metadata. `mipLevelCount >= 1`; entries past it are undefined.
    // Level 0 mirrors `width / height / pitchBytes` above. Effects that don't
    // request mips see `mipLevelCount == 1` and can ignore these fields.
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
    // Canvas sampling contract: the source's top-left sits at (extX, extY)
    // inside the (canvasWidth x canvasHeight) output frame; p_src = p_out - ext.
    // Identity when canvas == layer (AE clip space). Must stay byte-equal with
    // prgpu's Rust FrameParams.
    public uint canvasWidth;
    public uint canvasHeight;
    public uint layerWidth;
    public uint layerHeight;
    public int  extX;
    public int  extY;
}
