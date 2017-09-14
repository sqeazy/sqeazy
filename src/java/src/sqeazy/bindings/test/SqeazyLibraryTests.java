package sqeazy.bindings.test;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.IOException;

import org.bridj.CLong;
import org.bridj.Pointer;
import org.junit.Test;

import sqeazy.bindings.SqeazyLibrary;

public class SqeazyLibraryTests
{

    @Test
	public void testSQY_VERSION(){
		final Pointer<Integer> version = Pointer.allocateInts(3);
		SqeazyLibrary.SQY_Version_Triple(version);
		assertTrue(version.get(0) >= 0);
		assertTrue(version.get(1) >= 3);
		assertTrue(version.get(2) >= 0);
    }

    @Test
	public void testLZ4(){
		final int lBufferLength = 1024;

		final Pointer<Byte> lSourceBytes = Pointer.allocateBytes(lBufferLength);
		for (int i = 0; i < lBufferLength; i++)
		{
			lSourceBytes.set(i, (byte) (i));
		}

		final Pointer<Byte> lCompressedBytes = Pointer.allocateBytes((long) (lBufferLength * 1.1));
		final Pointer<CLong> lPointerToDestinationLength = Pointer.allocateCLong();
		lPointerToDestinationLength.setCLong((long) (lBufferLength * 1.1));
		final Pointer<CLong> lSourceShape = Pointer.pointerToCLongs(1,
																	1,
																	lBufferLength/2);
		final Pointer<Byte> bPipelineName = Pointer.pointerToCString("lz4");
		assertEquals(	0,
						SqeazyLibrary.SQY_PipelineEncode_UI16(bPipelineName,
															  lSourceBytes,
															  lSourceShape,
															  3,
															  lCompressedBytes,
															  lPointerToDestinationLength,
															  1)
			);

		final long lCompresssedBufferLength = lPointerToDestinationLength.getCLong();

		assertTrue(lCompresssedBufferLength != 0);
		assertTrue(lCompresssedBufferLength < lSourceBytes.getValidBytes());

		assertEquals(	0,
						SqeazyLibrary.SQY_Pipeline_Decompressed_Length(lCompressedBytes,
																	   lPointerToDestinationLength));

		assertEquals(	lBufferLength,
						lPointerToDestinationLength.getCLong());

		final Pointer<Byte> lDecodedBytes = Pointer.allocateBytes(lBufferLength);

		assertEquals(	0,
						SqeazyLibrary.SQY_PipelineDecode_UI16(lCompressedBytes,
															  lCompresssedBufferLength,
															  lDecodedBytes,
															  1));

		assertArrayEquals(lSourceBytes.getBytes(lBufferLength),
						  lDecodedBytes.getBytes(lBufferLength));

	}

    @Test
    public void testH5_RoundTRIP() throws IOException
    {
	final String lPipeline = "bitswap1->lz4";
	final int lWidth = 7;
	final int lHeight = 9;
	final int lDepth = 11;

	final int lBufferLengthInShorts = lWidth * lHeight * lDepth;// each side is
	// 8 pixels long

	final Pointer<Short> lSourceShort = Pointer.allocateShorts(lBufferLengthInShorts);
	for (int i = 0; i < lBufferLengthInShorts; i++)
	    {
		lSourceShort.set(i, (short) (i));
	    }

	final Pointer<Integer> lSourceShape = Pointer.pointerToInts(lWidth,
								    lHeight,
								    lDepth);

	// final String sFileName = "java_test.h5";
	// final String sDataset = "ramp";

	final File lTempFile = File.createTempFile(	SqeazyLibraryTests.class.getSimpleName(),
							"testH5_RoundTRIP.h5");
	System.out.println(lTempFile);
	lTempFile.delete();

	final Pointer<Byte> pFileName = Pointer.pointerToCString(lTempFile.getAbsolutePath());
	final Pointer<Byte> pDatasetName = Pointer.pointerToCString("ramp");

	assertEquals(	0,
			SqeazyLibrary.SQY_h5_write_UI16(pFileName,
							pDatasetName,
							lSourceShort,
							3,
							lSourceShape,
							Pointer.pointerToCString(lPipeline)));

	final Pointer<Integer> lDimension = Pointer.allocateInt();

	assertEquals(0, SqeazyLibrary.SQY_h5_query_ndims(	pFileName,
								pDatasetName,
								lDimension));

	assertEquals(3, lDimension.get(), 0.1);

	final Pointer<Integer> lQueriedShape = Pointer.allocateInts(lDimension.get());

	assertEquals(0, SqeazyLibrary.SQY_h5_query_shape(	pFileName,
								pDatasetName,
								lQueriedShape));

	assertEquals(lWidth, lQueriedShape.get(0), 0.1);
	assertEquals(lHeight, lQueriedShape.get(1), 0.1);
	assertEquals(lDepth, lQueriedShape.get(2), 0.1);

	final Pointer<Integer> lSizeofElement = Pointer.allocateInts(lDimension.get());
	assertEquals(0, SqeazyLibrary.SQY_h5_query_sizeof(pFileName,
							  pDatasetName,
							  lSizeofElement));

	assertEquals(2, lSizeofElement.get(), 0.1);

	final Pointer<Short> lReloadedShort = Pointer.allocateShorts(lBufferLengthInShorts);

	assertEquals(0, SqeazyLibrary.SQY_h5_read_UI16(	pFileName,
							pDatasetName,
							lReloadedShort));

	assertArrayEquals(lSourceShort.getShorts(lBufferLengthInShorts),
			  lReloadedShort.getShorts(lBufferLengthInShorts));

    }


    @Test
    public void testpipeline_RoundTRIP() throws IOException
    {
	final String lPipeline = "quantiser->h264";
	final Pointer<Byte> bPipelineName = Pointer.pointerToCString(lPipeline);
	    
	final int lWidth  = 128;
	final int lHeight = 128;
	final int lDepth  = 256;

	final int lBufferLengthInShorts = lWidth * lHeight * lDepth;
	final long lBufferLengthInByte = lBufferLengthInShorts*2;

	final Pointer<Short> lSourceShort = Pointer.allocateShorts(lBufferLengthInShorts);
	final Pointer<Short> lDestShort = Pointer.allocateShorts(lBufferLengthInShorts);
	for (int i = 0; i < lBufferLengthInShorts; i++)
	    {
		lSourceShort.set(i, (short) (1 << (i % 8)));
		lDestShort.set(i, (short) (0));
	    }

	final Pointer<CLong> lSourceShape = Pointer.pointerToCLongs(lDepth,
								    lHeight,
								    lWidth);
	
	assertEquals(	true,
			SqeazyLibrary.SQY_Pipeline_Possible(bPipelineName)
			);
	
	final Pointer<CLong> lMaxEncodedBytes = Pointer.allocateCLong();
	lMaxEncodedBytes.setCLong(lBufferLengthInByte);
	assertEquals(0,SqeazyLibrary.SQY_Pipeline_Max_Compressed_Length_UI16(bPipelineName,lMaxEncodedBytes));

	final long nil = 0;
	assertEquals( true, lMaxEncodedBytes.get().longValue()>nil);


	final long received_max_encoded_size = lMaxEncodedBytes.get().longValue();
	assertTrue( received_max_encoded_size>lBufferLengthInByte);
	final Pointer<Byte> bCompressedData = Pointer.allocateBytes(received_max_encoded_size);
	final Pointer<Byte> bInputData = lSourceShort.as(Byte.class);
	final Pointer<CLong> lEncodedBytes = Pointer.allocateCLong();
	assertEquals(0,
		     SqeazyLibrary.SQY_PipelineEncode_UI16(bPipelineName,
							   bInputData,
							   lSourceShape,
							   3,
							   bCompressedData,
							   lEncodedBytes)
		     );

	assertTrue(lEncodedBytes.getLong()>nil);
	assertTrue(lEncodedBytes.getLong()<lBufferLengthInByte);

	final Pointer<Byte> bDestShort = lDestShort.as(Byte.class);
	assertEquals(0,
		     SqeazyLibrary.SQY_PipelineDecode_UI16(bCompressedData,
							   lEncodedBytes.getCLong(),
							   bDestShort)
		     );
    }
}
