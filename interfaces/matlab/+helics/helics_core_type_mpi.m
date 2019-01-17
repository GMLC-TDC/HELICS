function v = helics_core_type_mpi()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812624);
  end
  v = vInitialized;
end
