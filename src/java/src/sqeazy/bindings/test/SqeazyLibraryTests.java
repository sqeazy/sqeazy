package sqeazy.bindings.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertTrue;

import org.bridj.CLong;
import org.bridj.Pointer;
import org.junit.Test;

import sqeazy.bindings.SqeazyLibrary;

public class SqeazyLibraryTests
{

    @Test
    public void testSQY_VERSION()
    {
	final Pointer<Integer> version = Pointer.allocateInts((long)3);
	SqeazyLibrary.SQY_Version_Triple(version);
	assertEquals((Integer)2015,version.get(0));
	assertEquals((Integer)4,version.get(1));
	assertEquals((Integer)28,version.get(2));
    }

    @Test
    public void testLZ4()
    {
	final int lBufferLength = 1024;

	final Pointer<Byte> lSourceBytes = Pointer.allocateBytes(lBufferLength);
	for (int i = 0; i < lBufferLength; i++){
	    lSourceBytes.set(i, (byte) (i));
	}

	final Pointer<Byte> lCompressedBytes = Pointer.allocateBytes((long) (lBufferLength * 1.1));
	final Pointer<CLong> lPointerToDestinationLength = Pointer.allocateCLong();
	lPointerToDestinationLength.setCLong((long)(lBufferLength * 1.1));
		
	assertEquals(	0,
			SqeazyLibrary.SQY_LZ4Encode(lSourceBytes,
						    lSourceBytes.getValidBytes(),
						    lCompressedBytes,
						    lPointerToDestinationLength));

	final long lCompresssedBufferLength = lPointerToDestinationLength.getCLong();

	assertTrue(lCompresssedBufferLength != (long)0);
	assertTrue(lCompresssedBufferLength < lSourceBytes.getValidBytes());

	assertEquals(	0,
			SqeazyLibrary.SQY_LZ4_Decompressed_Length(lCompressedBytes,
								  lPointerToDestinationLength));

	assertEquals(	lBufferLength,
			lPointerToDestinationLength.getCLong());

	final Pointer<Byte> lDecodedBytes = Pointer.allocateBytes(lBufferLength);

	assertEquals(0, SqeazyLibrary.SQY_LZ4Decode(lCompressedBytes,
						    lCompresssedBufferLength,
						    lDecodedBytes));

	assertArrayEquals(lSourceBytes.getBytes(lBufferLength),
			  lDecodedBytes.getBytes(lBufferLength));

    }

    @Test
    public void testH5_RoundTRIP()
    {
	final int lBufferLength = 512;//each side is 8 pixels long

	final Pointer<Short> lSourceShort = Pointer.allocateShorts(lBufferLength);
	for (int i = 0; i < lBufferLength; i++){
	    lSourceShort.set(i, (short) (i));
	}

	final Pointer<Integer> lSourceShape = Pointer.allocateInts(3);
	for (int i = 0; i < 3; i++){
	    lSourceShape.set(i, 8);
	}
	
	// final String sFileName = "java_test.h5";
	// final String sDataset = "ramp";

	final Pointer<Byte> pFileName = Pointer.pointerToCString("java_test.h5");
	final Pointer<Byte> pDataset  = Pointer.pointerToCString("ramp");
		
	assertEquals(	0,
			SqeazyLibrary.SQY_h5_write_UI16(pFileName,
							pDataset,
							lSourceShort,
							3,
							lSourceShape,
							Pointer.pointerToCString("ushort_bswap1_lz4")));

	final Pointer<Short> lReloadedShort = Pointer.allocateShorts(lBufferLength);

	assertEquals(	0,
			SqeazyLibrary.SQY_h5_read_UI16(pFileName,
							pDataset,
							lReloadedShort));
	
	assertArrayEquals(lSourceShort.getShorts(lBufferLength),
			  lReloadedShort.getShorts(lBufferLength));

    }

}
