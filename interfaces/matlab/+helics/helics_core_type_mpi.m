function v = helics_core_type_mpi()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 2);
  end
  v = vInitialized;
end
