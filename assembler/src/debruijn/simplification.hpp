/*
 * simplification.hpp
 *
 *  Created on: 1 Sep 2011
 *      Author: valery
 */

#pragma once

#include "standard.hpp"
#include "construction.hpp"
#include "omni_labelers.hpp"

namespace debruijn_graph {
void simplify_graph(PairedReadStream& stream, conj_graph_pack& gp,
		paired_info_index& paired_index);
} // debruijn_graph


// move impl to *.cpp
namespace debruijn_graph {

void simplify_graph(PairedReadStream& stream, conj_graph_pack& gp,
		paired_info_index& paired_index) {
	using namespace omnigraph;

	total_labeler_graph_struct graph_struct(gp.g, &gp.int_ids, &gp.edge_pos);
	total_labeler tot_lab(&graph_struct);

	exec_construction(stream, gp, tot_lab, paired_index);
	INFO("STAGE == Simplifying graph");

	// by single_cell 3->10
	SimplifyGraph<K> (gp, paired_index, tot_lab, 10, cfg::get().output_dir/*, etalon_paired_index*/);

	// by single_cell
	EdgeQuality<Graph> quality_labeler(gp.g, gp.index, gp.genome);
	LabelerList<Graph> labeler_list(tot_lab, quality_labeler);

	WriteGraphComponents<K> (gp.g, gp.index, labeler_list, gp.genome,
			cfg::get().output_dir + "graph_components/", "graph.dot",
			"graph_component", cfg::get().ds.IS);

	//  ProduceInfo<k>(g, index, *totLab, genome, output_folder + "simplified_graph.dot", "simplified_graph");

	//experimental
	if (cfg::get().paired_mode && cfg::get().late_paired_info) {
		FillPairedIndexWithReadCountMetric<K> (gp.g, gp.index, gp.kmer_mapper,
				paired_index, stream);
	}
	//experimental

	//experimental
//	if (cfg::get().paired_mode) {
//		INFO("Pair info aware ErroneousConnectionsRemoval");
//		RemoveEroneousEdgesUsingPairedInfo(gp.g, paired_index);
//		INFO("Pair info aware ErroneousConnectionsRemoval stats");
//		CountStats<K>(gp.g, gp.index, gp.genome);
//	}
	//experimental

	//	ProduceDetailedInfo<k>(g, index, labeler, genome, output_folder + "with_pair_info_edges_removed/",	"graph.dot", "no_erroneous_edges_graph");

	//  WriteGraphComponents<k>(g, index, *totLab, genome, output_folder + "graph_components" + "/", "graph.dot",
	//            "graph_component", cfg::get().ds.IS);

	//  number_of_components = PrintGraphComponents(output_folder + "graph_components/graph", g,
	//            cfg::get().ds.IS, int_ids, paired_index, EdgePos);
}

void load_simplification(conj_graph_pack& gp, paired_info_index& paired_index,
		files_t* used_files) {
	fs::path p = fs::path(cfg::get().load_from) / "simplified_graph";
	used_files->push_back(p);

	// TODO: what's the difference with construction?
	scanConjugateGraph(&gp.g, &gp.int_ids, p.string(), &paired_index,
			&gp.edge_pos, &gp.etalon_paired_index);

	scanKmerMapper(gp.g, gp.int_ids, p.string(), &gp.kmer_mapper);
}

void save_simplification(conj_graph_pack& gp, paired_info_index& paired_index) {
	fs::path p = fs::path(cfg::get().output_saves) / "simplified_graph";
	printGraph(gp.g, gp.int_ids, p.string(), paired_index, gp.edge_pos,
			&gp.etalon_paired_index);

	printKmerMapper(gp.g, gp.int_ids, p.string(), gp.kmer_mapper);

	//DEBUG
	//	KmerMapper<K+1, Graph> cmp(gp.g);
	//	scanKmerMapper(gp.g, gp.int_ids, p.string(), &cmp);
	//	if (!gp.kmer_mapper.CompareTo(cmp)) {
	//		WARN("Kmer mappers are not equal!");
	//	}
	//	else {
	//		INFO("Mappers are equal");
	//	}
	//DEBUG


	//todo temporary solution!!!
	OutputContigs(gp.g, cfg::get().output_root + "tmp_contigs.fasta");
}

void exec_simplification(PairedReadStream& stream, conj_graph_pack& gp,
		paired_info_index& paired_index) {
	if (cfg::get().entry_point <= ws_simplification) {
		simplify_graph(stream, gp, paired_index);
		save_simplification(gp, paired_index);
	} else {
		INFO("Loading Simplification");

		files_t used_files;
		load_simplification(gp, paired_index, &used_files);
		copy_files(used_files);
	}
}

} //debruijn_graph
