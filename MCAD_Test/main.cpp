#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <NCollection_List.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


const char* shapeTypeName(const TopAbs_ShapeEnum type)
{
	switch (type)
	{
	case TopAbs_SOLID:
		return "SOLID";
	case TopAbs_FACE:
		return "FACE";
	case TopAbs_WIRE:
		return "WIRE";
	case TopAbs_EDGE:
		return "EDGE";
	case TopAbs_VERTEX:
		return "VERTEX";
	default:
		return "UNKNOWN";
	}
}


TopoDS_Wire makeClosedWire(const std::vector<gp_Pnt>& points)
{
	if (points.size() < 3)
	{
		throw std::invalid_argument(
			"A closed contour requires at least three points");
	}

	BRepBuilderAPI_MakePolygon wireMaker;

	for (std::size_t i = 0; i < points.size(); ++i)
	{
		wireMaker.Add(points[i]);

		if (i > 0 && !wireMaker.Added())
		{
			throw std::invalid_argument(
				"The contour contains two consecutive coincident points");
		}
	}

	wireMaker.Close();

	if (!wireMaker.IsDone())
	{
		throw std::runtime_error(
			"Failed to build closed polygonal wire");
	}

	return wireMaker.Wire();
}


TopoDS_Face makeFaceFromWire(const TopoDS_Wire& wire)
{
	BRepBuilderAPI_MakeFace faceMaker(wire, true);

	if (!faceMaker.IsDone())
	{
		throw std::runtime_error(
			"Failed to build face from the closed wire");
	}

	return faceMaker.Face();
}


TopoDS_Shape makeTestShape1()
{
	const gp_Pnt point1(0., 0., 0.);
	const gp_Pnt point2(10., 0., 0.);
	const gp_Pnt point3(10., 10., 0.);
	const gp_Pnt point4(0., 10., 0.);

	const auto wire = makeClosedWire({ point1, point2, point3, point4 });
	const auto face = makeFaceFromWire(wire);

	const double height = 10.;
	const gp_Vec extrusionVector(0., 0., height);

	BRepPrimAPI_MakePrism extruder(face, extrusionVector);

	if (!extruder.IsDone())
	{
		throw std::runtime_error(
			"Failed to build the first extrusion");
	}

	const TopoDS_Shape solid = extruder.Shape();

	BRepCheck_Analyzer analyzer(solid);

	if (!analyzer.IsValid())
	{
		throw std::runtime_error(
			"The first extruded solid contains invalid topology");
	}

	return solid;
}


TopoDS_Shape makeTestShape2()
{
	const gp_Pnt point1(15., 2., 12.);
	const gp_Pnt point2(15., 8., 12.);
	const gp_Pnt point3(15., 8., 6.);
	const gp_Pnt point4(15., 2., 6.);

	const auto wire = makeClosedWire({ point1, point2, point3, point4 });
	const auto face = makeFaceFromWire(wire);

	const double height = 8.;
	const gp_Vec extrusionVector(-height, 0., 0.);

	BRepPrimAPI_MakePrism extruder(face, extrusionVector);

	if (!extruder.IsDone())
	{
		throw std::runtime_error(
			"Failed to build the second extrusion");
	}

	const TopoDS_Shape solid = extruder.Shape();

	BRepCheck_Analyzer analyzer(solid);

	if (!analyzer.IsValid())
	{
		throw std::runtime_error(
			"The second extruded solid contains invalid topology");
	}

	return solid;
}


void printHistoryForFaces(
	const TopoDS_Shape& sourceShape,
	BRepAlgoAPI_Cut& cutter,
	const std::string& shapeName)
{
	std::size_t faceIndex = 0;

	for (TopExp_Explorer explorer(sourceShape, TopAbs_FACE);
		explorer.More();
		explorer.Next())
	{
		++faceIndex;

		const TopoDS_Shape& oldFace = explorer.Current();

		const auto& modified = cutter.Modified(oldFace);
		const auto& generated = cutter.Generated(oldFace);
		const bool deleted = cutter.IsDeleted(oldFace);

		std::cout << shapeName << ", face " << faceIndex << '\n';

		std::cout << "  deleted: " << std::boolalpha << deleted << '\n';
		std::cout << "  modified: " << modified.Size() << '\n';
		std::cout << "  generated: " << generated.Size() << '\n';

		std::size_t index = 0;

		for (const TopoDS_Shape& shape : modified)
		{
			std::cout
				<< "\tmodified " << index++ << ", type = "
				<< shapeTypeName(shape.ShapeType())
				<< '\n';
		}

		index = 0;

		for (const TopoDS_Shape& shape : generated)
		{
			std::cout
				<< "\tgenerated " << index++ << ", type = "
				<< shapeTypeName(shape.ShapeType())
				<< '\n';
		}

		std::cout << '\n';
	}
}


int main()
{
	const auto shape1 = makeTestShape1();
	const auto shape2 = makeTestShape2();

	NCollection_List<TopoDS_Shape> arguments;
	arguments.Append(shape1);

	NCollection_List<TopoDS_Shape> tools;
	tools.Append(shape2);

	BRepAlgoAPI_Cut cutter;
	cutter.SetArguments(arguments);
	cutter.SetTools(tools);
	cutter.SetToFillHistory(true);
	cutter.Build();

	if (cutter.HasErrors() || !cutter.IsDone())
	{
		std::ostringstream errors;
		cutter.DumpErrors(errors);

		throw std::runtime_error(
			"Failed to perform Boolean Cut:\n" + errors.str());
	}

	if (cutter.HasWarnings())
	{
		std::cerr << "Warnings occurred while performing Boolean Cut:\n";

		cutter.DumpWarnings(std::cerr);
	}

	const TopoDS_Shape result = cutter.Shape();

	if (result.IsNull())
	{
		throw std::runtime_error(
			"Boolean Cut completed without producing result");
	}

	BRepCheck_Analyzer resultAnalyzer(result);

	if (!resultAnalyzer.IsValid())
	{
		throw std::runtime_error(
			"The Boolean Cut result contains invalid topology");
	}

	printHistoryForFaces(shape1, cutter, "Base solid");
	printHistoryForFaces(shape2, cutter, "Tool solid");

	if (cutter.HasHistory())
	{
		const auto history = cutter.History();

		if (!history.IsNull())
		{
			std::cout << "\nFull Boolean history:\n";
			history->Dump(std::cout);
		}
	}

	return 0;
}