function v = helics_iteration_result_halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812712);
  end
  v = vInitialized;
end
