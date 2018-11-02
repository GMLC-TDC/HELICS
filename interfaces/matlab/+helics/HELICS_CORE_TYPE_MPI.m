function v = HELICS_CORE_TYPE_MPI()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856920);
  end
  v = vInitialized;
end
