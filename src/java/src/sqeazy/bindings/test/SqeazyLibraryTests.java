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
	public void testSQY_VERSION()
	{
		final Pointer<Integer> version = Pointer.allocateInts(3);
		SqeazyLibrary.SQY_Version_Triple(version);
		assertEquals((Integer) 2015, version.get(0));
		assertEquals((Integer) 4, version.get(1));
		assertEquals((Integer) 28, version.get(2));
	}

	@Test
	public void testLZ4()
	{
		final int lBufferLength = 1024;

		final Pointer<Byte> lSourceBytes = Pointer.allocateBytes(lBufferLength);
		for (int i = 0; i < lBufferLength; i++)
		{
			lSourceBytes.set(i, (byte) (i));
		}

		final Pointer<Byte> lCompressedBytes = Pointer.allocateBytes((long) (lBufferLength * 1.1));
		final Pointer<CLong> lPointerToDestinationLength = Pointer.allocateCLong();
		lPointerToDestinationLength.setCLong((long) (lBufferLength * 1.1));

		assertEquals(	0,
									SqeazyLibrary.SQY_LZ4Encode(lSourceBytes,
																							lSourceBytes.getValidBytes(),
																							lCompressedBytes,
																							lPointerToDestinationLength));

		final long lCompresssedBufferLength = lPointerToDestinationLength.getCLong();

		assertTrue(lCompresssedBufferLength != 0);
		assertTrue(lCompresssedBufferLength < lSourceBytes.getValidBytes());

		assertEquals(	0,
									SqeazyLibrary.SQY_LZ4_Decompressed_Length(lCompressedBytes,
																														lPointerToDestinationLength));

		assertEquals(	lBufferLength,
									lPointerToDestinationLength.getCLong());

		final Pointer<Byte> lDecodedBytes = Pointer.allocateBytes(lBufferLength);

		assertEquals(	0,
									SqeazyLibrary.SQY_LZ4Decode(lCompressedBytes,
																							lCompresssedBufferLength,
																							lDecodedBytes));

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

}
