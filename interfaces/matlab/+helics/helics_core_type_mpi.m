function v = helics_core_type_mpi()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183034);
  end
  v = vInitialized;
end
