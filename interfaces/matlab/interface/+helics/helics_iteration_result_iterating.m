function v = helics_iteration_result_iterating()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 131);
  end
  v = vInitialized;
end
