from pathlib import Path

def transform_lsfrp(instance_name: str) -> None:

    '''We want to copy part of .sto file with:
        xEqq1370715  FlowofEquipmentq1370701          -1.0
        xEqq1370715  FlowofEquipmentq1371501           1.0
        xEqq1370725  FlowofEquipmentq1370701          -1.0
        xEqq1370725  FlowofEquipmentq1372501           1.0
        xEqq1370803  FlowofEquipmentq1370301           1.0
        xEqq1370803  FlowofEquipmentq1370801          -1.0
        ...
    to .cor file.
    COLUMNS section in .cor should be ordered by columns, otherwise, duplicate name error.
    Because of that we cannot simply copy the lines
    '''

    input_cor = f'{instance_name}.cor'
    cor_file = Path(input_cor)
    if not cor_file.exists():
        raise FileNotFoundError(f'{input_cor} file was not found')
    cor_lines = cor_file.read_text().splitlines()
    cor_start = None
    cor_end = None
    for cor_line_index, cor_line in enumerate(cor_lines):
        if 'COLUMNS' in cor_line:
            cor_start = cor_line_index+1
        elif 'RHS' in cor_line:
            assert(cor_start is not None and 'We assume that RHS is the next section after COLUMNS')
            cor_end = cor_line_index
            break
    if cor_start is None:
        raise ValueError(f'COLUMNS section not found in {input_cor}')
    if cor_end is None:
        raise ValueError(f'RHS section not found in {input_cor}')

    input_sto = f'{instance_name}.sto'
    sto_file = Path(input_sto)
    if sto_file.exists():
        sto_lines = sto_file.read_text().splitlines()
        sto_start = None
        sto_end = None
        for sto_line_index, sto_line in enumerate(sto_lines):
            if 'FlowofEquipmentq' in sto_line and sto_start is None:
                sto_start = sto_line_index
            elif 'FlowofEquipmentq' in sto_line:
                continue
            elif 'SC Scen2' in sto_line:
                assert(sto_start is not None and 'We assume that SC Scen2 is the next section after FlowofEquipmentq data')
                sto_end = sto_line_index
                break
            else:
                assert (sto_start is None and sto_end is None and 'We assume that FlowofEquipmentq is the last info for scenario and SC Scen2 goes afterwards')
        if sto_start is None:
            raise ValueError(f'FlowofEquipmentq info not found in {input_sto}')
        if sto_end is None:
            raise ValueError(f'SC Scen2 section not found in {input_sto}')

        sto_line_index = sto_start
        while sto_line_index < sto_end:
            sto_column_name = sto_lines[sto_line_index].split()[0]
            cor_line_index = cor_start
            while cor_line_index < cor_end: # no guarantee that lines in .cor are ordered => restart for every sto_line
                cor_column_name = cor_lines[cor_line_index].split()[0]
                if cor_column_name == sto_column_name:
                    cor_lines.insert(cor_line_index, sto_lines[sto_line_index])
                    cor_end += 1 # inserted a line => COLUMNS ends one lines later
                    sto_line_index += 1
                    print(f'Processed {sto_line_index-sto_start}/{sto_end-sto_start} lines in {input_sto}')
                    break
                else:
                    cor_line_index += 1
            else:
                raise ValueError(f'Data related to {sto_column_name} was not found in {input_cor} COLUMNS section')

        cor_lines_with_endl = [line + '\n' for line in cor_lines]
        output_cor = f'fixed_{input_cor}'
        with open(output_cor, 'w') as sto_file_fixed:
            sto_file_fixed.writelines(cor_lines_with_endl)
        print(f'Saved {output_cor}')
    else:
        raise FileNotFoundError(f'{input_sto} file was not found')

def main():
    transform_lsfrp('16LSFRP')

if __name__ == '__main__':
    main()