function v = helics_iteration_result_next_step()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 116);
  end
  v = vInitialized;
end
